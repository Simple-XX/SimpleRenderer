use minifb::{Key, Window, WindowOptions};

pub struct Display {
    window: Window,
    width: usize,
    height: usize,
    // Pre-allocated buffer for format conversion
    minifb_buffer: Vec<u32>,
}

impl Display {
    pub fn new(width: usize, height: usize) -> Self {
        let window = Window::new(
            "SimpleRenderer (Rust)",
            width,
            height,
            WindowOptions::default(),
        )
        .expect("Failed to create window");

        Self {
            window,
            width,
            height,
            minifb_buffer: vec![0u32; width * height],
        }
    }

    pub fn is_open(&self) -> bool {
        self.window.is_open()
            && !self.window.is_key_down(Key::Escape)
            && !self.window.is_key_down(Key::Q)
    }

    /// Convert our RGBA u32 format to minifb's 0x00RRGGBB format and display.
    pub fn update(&mut self, buffer: &[u32]) {
        // Our Color u32 format: R in bits 0-7, G in bits 8-15, B in bits 16-23, A in bits 24-31
        // minifb expects: 0x00RRGGBB = R in bits 16-23, G in bits 8-15, B in bits 0-7
        for (i, &pixel) in buffer.iter().enumerate() {
            let r = pixel & 0xFF;
            let g = (pixel >> 8) & 0xFF;
            let b = (pixel >> 16) & 0xFF;
            self.minifb_buffer[i] = (r << 16) | (g << 8) | b;
        }
        self.window
            .update_with_buffer(&self.minifb_buffer, self.width, self.height)
            .expect("Failed to update window");
    }

    pub fn handle_input(&self, camera: &mut super::camera::Camera) {
        if self.window.is_key_down(Key::Up) {
            camera.move_up(1.0);
        }
        if self.window.is_key_down(Key::Down) {
            camera.move_up(-1.0);
        }
        if self.window.is_key_down(Key::Left) {
            camera.move_right(-1.0);
        }
        if self.window.is_key_down(Key::Right) {
            camera.move_right(1.0);
        }
    }
}
