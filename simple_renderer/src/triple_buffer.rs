use std::cell::UnsafeCell;
use std::sync::atomic::{AtomicU8, Ordering};
use std::sync::Arc;

use crate::color::Color;

// State encoding in AtomicU8:
//   bits [1:0] = back buffer index (0, 1, or 2)
//   bit  [2]   = fresh flag (1 = back contains a new frame not yet consumed)
const FRESH_BIT: u8 = 0b100;
const INDEX_MASK: u8 = 0b011;

struct SharedBuffers {
    bufs: [UnsafeCell<Vec<u32>>; 3],
    state: AtomicU8,
    width: usize,
    height: usize,
}

// Safety: at any instant each buffer is accessed by at most one thread.
// The atomic state variable ensures the writer and reader never touch
// the same buffer simultaneously.
unsafe impl Sync for SharedBuffers {}

pub struct TripleBufferWriter {
    shared: Arc<SharedBuffers>,
    render_idx: u8,
}

// Safety: TripleBufferWriter is only used from the render thread.
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
        // Initial assignment: front=0, back=1, render=2
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
        // Safety: render_idx is exclusively owned by this thread
        // and never equals back_idx or front_idx at this point.
        unsafe { &mut *self.shared.bufs[self.render_idx as usize].get() }
    }

    pub fn clear(&mut self, color: Color) {
        let val: u32 = color.into();
        self.render_buffer_mut().fill(val);
    }

    /// Publish the completed render buffer as the new back buffer.
    /// Returns the old back buffer index (now the writer's new render target).
    /// This never blocks — if the reader hasn't consumed the previous back
    /// buffer, it gets overwritten (dropped frame). This is triple-buffer
    /// semantics.
    pub fn publish(&mut self) {
        let new_state = (self.render_idx & INDEX_MASK) | FRESH_BIT;
        let old_state = self.shared.state.swap(new_state, Ordering::AcqRel);
        // Reclaim the old back buffer as our new render target.
        self.render_idx = old_state & INDEX_MASK;
    }

    /// Publish and then wait until the reader consumes the back buffer.
    /// This simulates double-buffer + VSync behavior where the GPU blocks
    /// until the display has swapped.
    pub fn publish_and_wait(&mut self) {
        self.publish();
        while self.shared.state.load(Ordering::Acquire) & FRESH_BIT != 0 {
            std::hint::spin_loop();
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
    /// If a new frame is available in the back buffer, swap it to front.
    /// Returns true if a new frame was obtained.
    pub fn update(&mut self) -> bool {
        let old_state = self.shared.state.load(Ordering::Acquire);
        if old_state & FRESH_BIT == 0 {
            return false; // no new frame
        }

        // Swap: our front becomes the new back, we take back as new front.
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
                // Writer published again between our load and CAS.
                // Retry once — the new back is even fresher.
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
                    Err(_) => false, // give up this tick, try next frame
                }
            }
        }
    }

    pub fn front_buffer(&self) -> &[u32] {
        // Safety: front_idx is exclusively owned by this thread.
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

// ── Tests ──────────────────────────────────────────────────────────────────

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
        assert!(!reader.update()); // no new frame
    }

    #[test]
    fn triple_buffer_overwrites_unconsumed_back() {
        let (mut writer, mut reader) = create_triple_buffer(2, 2);

        // Publish frame A (value 0xAA)
        writer.render_buffer_mut().fill(0xAA);
        writer.publish();

        // Publish frame B (value 0xBB) without reader consuming A
        writer.render_buffer_mut().fill(0xBB);
        writer.publish();

        // Reader should get the latest (B), not A
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

        // Give writer time to publish and start waiting
        std::thread::sleep(std::time::Duration::from_millis(10));

        // Consume the frame — this unblocks the writer
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
                // Value must be one of the published frames, not garbage
                assert!(val <= frame);
                // All pixels in the front buffer must be the same value
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
