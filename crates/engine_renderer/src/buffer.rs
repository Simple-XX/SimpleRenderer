// Copyright The SimpleGameEngine Contributors


use crate::color::Color;

/// 双缓冲帧缓冲区。
///
/// 维护两个 `Vec<u32>` 缓冲区。一个是*绘制*缓冲区（由渲染器写入），
/// 另一个是*显示*缓冲区（由显示子系统读取）。`swap` 只是切换两者的角色——
/// 不进行任何数据拷贝。
pub struct Buffer {
    width: usize,
    height: usize,
    framebuffer_1: Vec<u32>,
    framebuffer_2: Vec<u32>,
    draw_is_first: bool,
}

impl Buffer {
    /// 创建一个 `width × height` 像素的双缓冲帧缓冲区。
    /// 两个缓冲区均初始化为零。
    pub fn new(width: usize, height: usize) -> Self {
        let size = width * height;
        Self {
            width,
            height,
            framebuffer_1: vec![0u32; size],
            framebuffer_2: vec![0u32; size],
            draw_is_first: true,
        }
    }

    /// 用 `color` 填充绘制缓冲区的每个像素。
    pub fn clear_draw_buffer(&mut self, color: Color) {
        let val: u32 = color.into();
        self.draw_buffer_mut().fill(val);
    }

    /// 交换绘制缓冲区和显示缓冲区（不移动数据——只是翻转一个标志位）。
    pub fn swap(&mut self) {
        self.draw_is_first = !self.draw_is_first;
    }

    /// 当前绘制缓冲区的不可变视图。
    pub fn draw_buffer(&self) -> &[u32] {
        if self.draw_is_first {
            &self.framebuffer_1
        } else {
            &self.framebuffer_2
        }
    }

    /// 当前绘制缓冲区的可变视图。
    pub fn draw_buffer_mut(&mut self) -> &mut [u32] {
        if self.draw_is_first {
            &mut self.framebuffer_1
        } else {
            &mut self.framebuffer_2
        }
    }

    /// 当前显示缓冲区的不可变视图。
    pub fn display_buffer(&self) -> &[u32] {
        if self.draw_is_first {
            &self.framebuffer_2
        } else {
            &self.framebuffer_1
        }
    }

    /// 缓冲区宽度（像素）。
    #[inline]
    pub fn width(&self) -> usize {
        self.width
    }

    /// 缓冲区高度（像素）。
    #[inline]
    pub fn height(&self) -> usize {
        self.height
    }
}


#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn new_buffers_are_zeroed() {
        let buf = Buffer::new(4, 3);
        assert_eq!(buf.width(), 4);
        assert_eq!(buf.height(), 3);
        assert!(buf.draw_buffer().iter().all(|&p| p == 0));
        assert!(buf.display_buffer().iter().all(|&p| p == 0));
    }

    #[test]
    fn buffer_sizes_are_correct() {
        let buf = Buffer::new(10, 20);
        assert_eq!(buf.draw_buffer().len(), 200);
        assert_eq!(buf.display_buffer().len(), 200);
    }

    #[test]
    fn clear_draw_buffer_fills_with_color() {
        let mut buf = Buffer::new(2, 2);
        let red = Color::RED;
        let red_u32: u32 = red.into();
        buf.clear_draw_buffer(red);
        assert!(buf.draw_buffer().iter().all(|&p| p == red_u32));
        // 显示缓冲区应该仍然是零。
        assert!(buf.display_buffer().iter().all(|&p| p == 0));
    }

    #[test]
    fn swap_exchanges_draw_and_display() {
        let mut buf = Buffer::new(2, 2);
        let white = Color::WHITE;
        let white_u32: u32 = white.into();

        // 用白色填充绘制缓冲区。
        buf.clear_draw_buffer(white);
        assert!(buf.draw_buffer().iter().all(|&p| p == white_u32));
        assert!(buf.display_buffer().iter().all(|&p| p == 0));

        // 交换后，旧的绘制缓冲区变为显示缓冲区。
        buf.swap();
        assert!(buf.display_buffer().iter().all(|&p| p == white_u32));
        assert!(buf.draw_buffer().iter().all(|&p| p == 0));
    }

    #[test]
    fn double_swap_returns_to_original() {
        let mut buf = Buffer::new(3, 3);
        let blue = Color::BLUE;
        let blue_u32: u32 = blue.into();
        buf.clear_draw_buffer(blue);

        buf.swap();
        buf.swap();

        // 绘制缓冲区应该仍然包含我们写入的蓝色。
        assert!(buf.draw_buffer().iter().all(|&p| p == blue_u32));
    }

    #[test]
    fn draw_buffer_mut_allows_pixel_write() {
        let mut buf = Buffer::new(4, 4);
        buf.draw_buffer_mut()[5] = 0xDEADBEEF;
        assert_eq!(buf.draw_buffer()[5], 0xDEADBEEF);
    }

    #[test]
    fn zero_size_buffer() {
        let buf = Buffer::new(0, 0);
        assert_eq!(buf.draw_buffer().len(), 0);
        assert_eq!(buf.display_buffer().len(), 0);
    }
}
