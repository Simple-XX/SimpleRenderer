// Copyright The SimpleRenderer Contributors

use minifb::{Key, MouseButton, MouseMode, Window, WindowOptions};
use simple_renderer::RenderingMode;

use crate::camera::{Camera, CameraMovement};

pub enum InputAction {
    SetMode(RenderingMode),
    ToggleVSync,
    ToggleBufferMode,
}

pub struct Display {
    window: Window,
    width: usize,
    height: usize,
    minifb_buffer: Vec<u32>,
    last_mouse_x: f32,
    last_mouse_y: f32,
    right_dragging: bool,
}

impl Display {
    pub fn new(width: usize, height: usize) -> Self {
        let window = Window::new(
            "SimpleRenderer (Rust)",
            width,
            height,
            WindowOptions::default(),
        )
        .expect("failed to create window");

        Self {
            window,
            width,
            height,
            minifb_buffer: vec![0u32; width * height],
            last_mouse_x: width as f32 / 2.0,
            last_mouse_y: height as f32 / 2.0,
            right_dragging: false,
        }
    }

    pub fn is_open(&self) -> bool {
        self.window.is_open()
            && !self.window.is_key_down(Key::Escape)
            && !self.window.is_key_down(Key::Q)
    }

    /// RGBA → minifb 0x00RRGGBB
    pub fn update(&mut self, buffer: &[u32]) {
        for (i, &pixel) in buffer.iter().enumerate() {
            let r = pixel & 0xFF;
            let g = (pixel >> 8) & 0xFF;
            let b = (pixel >> 16) & 0xFF;
            self.minifb_buffer[i] = (r << 16) | (g << 8) | b;
        }
        self.window
            .update_with_buffer(&self.minifb_buffer, self.width, self.height)
            .expect("failed to update window buffer");
    }

    pub fn handle_input(&mut self, camera: &mut Camera, delta_time: f32) -> Option<InputAction> {
        if self.window.is_key_down(Key::W) || self.window.is_key_down(Key::Up) {
            camera.process_keyboard(CameraMovement::Forward, delta_time);
        }
        if self.window.is_key_down(Key::S) || self.window.is_key_down(Key::Down) {
            camera.process_keyboard(CameraMovement::Backward, delta_time);
        }
        if self.window.is_key_down(Key::A) || self.window.is_key_down(Key::Left) {
            camera.process_keyboard(CameraMovement::Left, delta_time);
        }
        if self.window.is_key_down(Key::D) || self.window.is_key_down(Key::Right) {
            camera.process_keyboard(CameraMovement::Right, delta_time);
        }
        if self.window.is_key_down(Key::Space) {
            camera.process_keyboard(CameraMovement::Up, delta_time);
        }
        if self.window.is_key_down(Key::LeftShift) {
            camera.process_keyboard(CameraMovement::Down, delta_time);
        }

        if let Some((_, scroll_y)) = self.window.get_scroll_wheel() {
            if scroll_y > 0.0 {
                camera.adjust_speed(1.1);
            } else if scroll_y < 0.0 {
                camera.adjust_speed(0.9);
            }
        }

        if let Some((mx, my)) = self.window.get_mouse_pos(MouseMode::Clamp) {
            if self.window.get_mouse_down(MouseButton::Right) {
                if self.right_dragging {
                    let dx = mx - self.last_mouse_x;
                    let dy = self.last_mouse_y - my;
                    camera.process_mouse(dx, dy);
                }
                self.last_mouse_x = mx;
                self.last_mouse_y = my;
                self.right_dragging = true;
            } else {
                self.right_dragging = false;
                self.last_mouse_x = mx;
                self.last_mouse_y = my;
            }
        }

        // is_key_pressed: triggers once per keypress (not held)
        if self.window.is_key_pressed(Key::V, minifb::KeyRepeat::No) {
            return Some(InputAction::ToggleVSync);
        }
        if self.window.is_key_pressed(Key::B, minifb::KeyRepeat::No) {
            return Some(InputAction::ToggleBufferMode);
        }

        if self.window.is_key_down(Key::Key1) {
            return Some(InputAction::SetMode(RenderingMode::PerTriangle));
        }
        if self.window.is_key_down(Key::Key2) {
            return Some(InputAction::SetMode(RenderingMode::TileBased));
        }
        if self.window.is_key_down(Key::Key3) {
            return Some(InputAction::SetMode(RenderingMode::Deferred));
        }
        if self.window.is_key_down(Key::Key4) {
            return Some(InputAction::SetMode(RenderingMode::TileBasedDeferred));
        }

        None
    }

    pub fn set_title(&mut self, title: &str) {
        self.window.set_title(title);
    }
}
