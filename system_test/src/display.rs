use minifb::{Key, MouseButton, MouseMode, Window, WindowOptions};
use simple_renderer::RenderingMode;

pub struct Display {
    window: Window,
    width: usize,
    height: usize,
    // Pre-allocated buffer for format conversion
    minifb_buffer: Vec<u32>,
    last_mouse_x: f32,
    last_mouse_y: f32,
    mouse_initialized: bool,
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
            last_mouse_x: 0.0,
            last_mouse_y: 0.0,
            mouse_initialized: false,
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

    pub fn handle_input(&mut self, camera: &mut super::camera::Camera) -> Option<RenderingMode> {
        // Keyboard movement
        if self.window.is_key_down(Key::W) || self.window.is_key_down(Key::Up) {
            camera.move_forward(0.05);
        }
        if self.window.is_key_down(Key::S) || self.window.is_key_down(Key::Down) {
            camera.move_forward(-0.05);
        }
        if self.window.is_key_down(Key::A) || self.window.is_key_down(Key::Left) {
            camera.move_right(-0.05);
        }
        if self.window.is_key_down(Key::D) || self.window.is_key_down(Key::Right) {
            camera.move_right(0.05);
        }
        if self.window.is_key_down(Key::Space) {
            camera.move_up(0.05);
        }
        if self.window.is_key_down(Key::LeftShift) {
            camera.move_up(-0.05);
        }

        // Mouse rotation (right mouse button + drag)
        if let Some((mx, my)) = self.window.get_mouse_pos(MouseMode::Clamp) {
            if self.window.get_mouse_down(MouseButton::Right) {
                if self.mouse_initialized {
                    let dx = mx - self.last_mouse_x;
                    let dy = my - self.last_mouse_y;
                    let sensitivity = 0.3;
                    camera.rotate(dx * sensitivity, -dy * sensitivity);
                }
                self.last_mouse_x = mx;
                self.last_mouse_y = my;
                self.mouse_initialized = true;
            } else {
                // Update position even when not pressing to avoid jumps
                self.last_mouse_x = mx;
                self.last_mouse_y = my;
            }
        }

        // Number keys for mode switching
        if self.window.is_key_down(Key::Key1) {
            return Some(RenderingMode::PerTriangle);
        }
        if self.window.is_key_down(Key::Key2) {
            return Some(RenderingMode::TileBased);
        }
        if self.window.is_key_down(Key::Key3) {
            return Some(RenderingMode::Deferred);
        }
        if self.window.is_key_down(Key::Key4) {
            return Some(RenderingMode::TileBasedDeferred);
        }

        None
    }

    pub fn set_title(&mut self, title: &str) {
        self.window.set_title(title);
    }
}
