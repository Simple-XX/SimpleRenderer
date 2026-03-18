use std::cell::UnsafeCell;
use std::sync::atomic::{AtomicU8, Ordering};
use std::sync::Arc;

use crate::color::Color;

// AtomicU8 中的状态编码：
//   位 [1:0] = 后台缓冲区索引（0、1 或 2）
//   位 [2]   = 新鲜标志（1 = 后台包含一帧尚未被消费的新帧）
const FRESH_BIT: u8 = 0b100;
const INDEX_MASK: u8 = 0b011;

struct SharedBuffers {
    bufs: [UnsafeCell<Vec<u32>>; 3],
    state: AtomicU8,
    width: usize,
    height: usize,
}

// 安全性：在任意时刻，每个缓冲区最多只被一个线程访问。
// 原子状态变量确保写入者和读取者不会同时触及同一个缓冲区。
unsafe impl Sync for SharedBuffers {}

pub struct TripleBufferWriter {
    shared: Arc<SharedBuffers>,
    render_idx: u8,
}

// 安全性：TripleBufferWriter 仅在渲染线程中使用。
unsafe impl Send for TripleBufferWriter {}

pub struct TripleBufferReader {
    shared: Arc<SharedBuffers>,
    front_idx: u8,
}

pub fn create_triple_buffer(
    width: usize,
    height: usize,
) -> (TripleBufferWriter, TripleBufferReader) {
    let size = width * height;
    let shared = Arc::new(SharedBuffers {
        bufs: [
            UnsafeCell::new(vec![0u32; size]),
            UnsafeCell::new(vec![0u32; size]),
            UnsafeCell::new(vec![0u32; size]),
        ],
        // 初始分配：front=0, back=1, render=2
        state: AtomicU8::new(1), // back_idx=1, fresh=false
        width,
        height,
    });

    let writer = TripleBufferWriter {
        shared: Arc::clone(&shared),
        render_idx: 2,
    };
    let reader = TripleBufferReader {
        shared,
        front_idx: 0,
    };
    (writer, reader)
}

impl TripleBufferWriter {
    pub fn render_buffer_mut(&mut self) -> &mut [u32] {
        // 安全性：render_idx 由本线程独占拥有，
        // 且在此时刻不会等于 back_idx 或 front_idx。
        unsafe { &mut *self.shared.bufs[self.render_idx as usize].get() }
    }

    pub fn clear(&mut self, color: Color) {
        let val: u32 = color.into();
        self.render_buffer_mut().fill(val);
    }

    /// 将已完成的渲染缓冲区发布为新的后台缓冲区。
    /// 返回旧的后台缓冲区索引（现在成为写入者的新渲染目标）。
    /// 此操作永不阻塞——如果读取者尚未消费上一个后台缓冲区，
    /// 它会被覆盖（丢帧）。这是三重缓冲的语义。
    pub fn publish(&mut self) {
        let new_state = (self.render_idx & INDEX_MASK) | FRESH_BIT;
        let old_state = self.shared.state.swap(new_state, Ordering::AcqRel);
        // 回收旧的后台缓冲区作为新的渲染目标。
        self.render_idx = old_state & INDEX_MASK;
    }

    /// 发布后等待读取者消费后台缓冲区。
    /// 这模拟了双缓冲 + VSync 的行为，即 GPU 阻塞直到
    /// 显示器完成交换。
    pub fn publish_and_wait(&mut self) {
        self.publish();
        // 混合等待：先短暂自旋以降低延迟，然后让出 CPU 以避免空转。
        const SPIN_ITERS: u32 = 64;
        let mut spins = 0u32;
        while self.shared.state.load(Ordering::Acquire) & FRESH_BIT != 0 {
            if spins < SPIN_ITERS {
                std::hint::spin_loop();
                spins += 1;
            } else {
                std::thread::yield_now();
            }
        }
    }

    #[inline]
    pub fn width(&self) -> usize {
        self.shared.width
    }

    #[inline]
    pub fn height(&self) -> usize {
        self.shared.height
    }
}

impl TripleBufferReader {
    /// 如果后台缓冲区有新帧可用，将其交换到前台。
    /// 返回 true 表示获取到了新帧。
    pub fn update(&mut self) -> bool {
        let old_state = self.shared.state.load(Ordering::Acquire);
        if old_state & FRESH_BIT == 0 {
            return false; // 没有新帧
        }

        // 交换：我们的前台变为新的后台，我们取后台作为新的前台。
        let new_state = self.front_idx & INDEX_MASK; // fresh=false
        match self.shared.state.compare_exchange(
            old_state,
            new_state,
            Ordering::AcqRel,
            Ordering::Acquire,
        ) {
            Ok(_) => {
                self.front_idx = old_state & INDEX_MASK;
                true
            }
            Err(current) => {
                // 在我们的 load 和 CAS 之间写入者又发布了一帧。
                // 重试一次——新的后台帧更新鲜。
                if current & FRESH_BIT == 0 {
                    return false;
                }
                let new_state = self.front_idx & INDEX_MASK;
                match self.shared.state.compare_exchange(
                    current,
                    new_state,
                    Ordering::AcqRel,
                    Ordering::Relaxed,
                ) {
                    Ok(_) => {
                        self.front_idx = current & INDEX_MASK;
                        true
                    }
                    Err(_) => false, // 本轮放弃，下一帧再试
                }
            }
        }
    }

    pub fn front_buffer(&self) -> &[u32] {
        // 安全性：front_idx 由本线程独占拥有。
        unsafe { &*self.shared.bufs[self.front_idx as usize].get() }
    }

    #[inline]
    pub fn width(&self) -> usize {
        self.shared.width
    }

    #[inline]
    pub fn height(&self) -> usize {
        self.shared.height
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::atomic::AtomicBool;

    #[test]
    fn create_returns_correct_dimensions() {
        let (writer, reader) = create_triple_buffer(100, 50);
        assert_eq!(writer.width(), 100);
        assert_eq!(writer.height(), 50);
        assert_eq!(reader.width(), 100);
        assert_eq!(reader.height(), 50);
    }

    #[test]
    fn initial_front_is_zeroed() {
        let (_writer, reader) = create_triple_buffer(4, 4);
        assert!(reader.front_buffer().iter().all(|&p| p == 0));
    }

    #[test]
    fn update_without_publish_returns_false() {
        let (_writer, mut reader) = create_triple_buffer(4, 4);
        assert!(!reader.update());
    }

    #[test]
    fn publish_then_update_delivers_frame() {
        let (mut writer, mut reader) = create_triple_buffer(4, 4);

        writer.render_buffer_mut().fill(0xDEADBEEF);
        writer.publish();

        assert!(reader.update());
        assert!(reader.front_buffer().iter().all(|&p| p == 0xDEADBEEF));
    }

    #[test]
    fn second_update_without_new_publish_returns_false() {
        let (mut writer, mut reader) = create_triple_buffer(4, 4);

        writer.render_buffer_mut().fill(1);
        writer.publish();
        assert!(reader.update());
        assert!(!reader.update()); // 没有新帧
    }

    #[test]
    fn triple_buffer_overwrites_unconsumed_back() {
        let (mut writer, mut reader) = create_triple_buffer(2, 2);

        // 发布帧 A（值 0xAA）
        writer.render_buffer_mut().fill(0xAA);
        writer.publish();

        // 在读取者未消费 A 的情况下发布帧 B（值 0xBB）
        writer.render_buffer_mut().fill(0xBB);
        writer.publish();

        // 读取者应该获取到最新的帧（B），而不是 A
        assert!(reader.update());
        assert!(reader.front_buffer().iter().all(|&p| p == 0xBB));
    }

    #[test]
    fn publish_and_wait_blocks_until_consumed() {
        let (mut writer, mut reader) = create_triple_buffer(2, 2);

        let writer_thread = std::thread::spawn(move || {
            writer.render_buffer_mut().fill(42);
            writer.publish_and_wait();
            writer
        });

        // 给写入者时间去发布并开始等待
        std::thread::sleep(std::time::Duration::from_millis(10));

        // 消费该帧——这会解除写入者的阻塞
        assert!(reader.update());
        assert!(reader.front_buffer().iter().all(|&p| p == 42));

        let _writer = writer_thread.join().unwrap();
    }

    #[test]
    fn many_frames_no_corruption() {
        let (mut writer, mut reader) = create_triple_buffer(8, 8);

        for frame in 0u32..100 {
            writer.render_buffer_mut().fill(frame);
            writer.publish();

            if reader.update() {
                let val = reader.front_buffer()[0];
                // 值必须是已发布帧中的某一帧，而不是垃圾数据
                assert!(val <= frame);
                // 前台缓冲区中所有像素的值必须相同
                assert!(reader.front_buffer().iter().all(|&p| p == val));
            }
        }
    }

    #[test]
    fn cross_thread_stress_test() {
        let (mut writer, mut reader) = create_triple_buffer(16, 16);
        let done = Arc::new(AtomicBool::new(false));
        let done_clone = Arc::clone(&done);

        let handle = std::thread::spawn(move || {
            for frame in 0u32..500 {
                writer.render_buffer_mut().fill(frame);
                writer.publish();
                std::thread::yield_now();
            }
            done_clone.store(true, std::sync::atomic::Ordering::Release);
        });

        let mut last_seen = 0u32;
        let mut frames_received = 0;
        while !done.load(std::sync::atomic::Ordering::Acquire) || reader.update() {
            if reader.update() {
                let val = reader.front_buffer()[0];
                assert!(val >= last_seen, "frames must not go backward");
                assert!(reader.front_buffer().iter().all(|&p| p == val));
                last_seen = val;
                frames_received += 1;
            }
            std::thread::yield_now();
        }

        handle.join().unwrap();

        assert!(frames_received > 0, "should have received at least 1 frame");
    }

    #[test]
    fn clear_fills_render_buffer() {
        let (mut writer, mut reader) = create_triple_buffer(2, 2);
        let red = Color::RED;
        let red_u32: u32 = red.into();

        writer.clear(red);
        writer.publish();
        reader.update();

        assert!(reader.front_buffer().iter().all(|&p| p == red_u32));
    }
}
