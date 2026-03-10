use crate::color::Color;

/// Double-buffered framebuffer.
///
/// Maintains two `Vec<u32>` buffers. One is the *draw* buffer (written to by
/// the renderer) and the other is the *display* buffer (read by the display
/// subsystem). `swap` simply toggles which is which — no data is copied.
pub struct Buffer {
    width: usize,
    height: usize,
    framebuffer_1: Vec<u32>,
    framebuffer_2: Vec<u32>,
    draw_is_first: bool,
}

impl Buffer {
    /// Create a new double-buffered framebuffer of `width × height` pixels.
    /// Both buffers are zero-initialised.
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

    /// Fill every pixel in the draw buffer with `color`.
    pub fn clear_draw_buffer(&mut self, color: Color) {
        let val: u32 = color.into();
        self.draw_buffer_mut().fill(val);
    }

    /// Swap draw and display buffers (no data movement — just flips a flag).
    pub fn swap(&mut self) {
        self.draw_is_first = !self.draw_is_first;
    }

    /// Immutable view of the current draw buffer.
    pub fn draw_buffer(&self) -> &[u32] {
        if self.draw_is_first {
            &self.framebuffer_1
        } else {
            &self.framebuffer_2
        }
    }

    /// Mutable view of the current draw buffer.
    pub fn draw_buffer_mut(&mut self) -> &mut [u32] {
        if self.draw_is_first {
            &mut self.framebuffer_1
        } else {
            &mut self.framebuffer_2
        }
    }

    /// Immutable view of the current display buffer.
    pub fn display_buffer(&self) -> &[u32] {
        if self.draw_is_first {
            &self.framebuffer_2
        } else {
            &self.framebuffer_1
        }
    }

    /// Buffer width in pixels.
    #[inline]
    pub fn width(&self) -> usize {
        self.width
    }

    /// Buffer height in pixels.
    #[inline]
    pub fn height(&self) -> usize {
        self.height
    }
}

// ── Tests ──────────────────────────────────────────────────────────────────

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
        // Display buffer should still be zeroed.
        assert!(buf.display_buffer().iter().all(|&p| p == 0));
    }

    #[test]
    fn swap_exchanges_draw_and_display() {
        let mut buf = Buffer::new(2, 2);
        let white = Color::WHITE;
        let white_u32: u32 = white.into();

        // Fill draw buffer with white.
        buf.clear_draw_buffer(white);
        assert!(buf.draw_buffer().iter().all(|&p| p == white_u32));
        assert!(buf.display_buffer().iter().all(|&p| p == 0));

        // After swap, the old draw buffer becomes display.
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

        // Draw buffer should still have the blue we wrote.
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
