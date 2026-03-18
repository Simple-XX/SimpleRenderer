// 鼠标输入状态管理

use std::collections::HashSet;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum MouseButton {
    Left,
    Right,
    Middle,
}

#[derive(Debug, Clone)]
pub struct MouseState {
    pub position: glam::Vec2,
    pub delta: glam::Vec2,
    pub scroll_delta: f32,
    pressed: HashSet<MouseButton>,
    just_pressed: HashSet<MouseButton>,
    just_released: HashSet<MouseButton>,
}

impl Default for MouseState {
    fn default() -> Self {
        Self {
            position: glam::Vec2::ZERO,
            delta: glam::Vec2::ZERO,
            scroll_delta: 0.0,
            pressed: HashSet::new(),
            just_pressed: HashSet::new(),
            just_released: HashSet::new(),
        }
    }
}

impl MouseState {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn is_pressed(&self, button: MouseButton) -> bool {
        self.pressed.contains(&button)
    }

    pub fn just_pressed(&self, button: MouseButton) -> bool {
        self.just_pressed.contains(&button)
    }

    pub fn just_released(&self, button: MouseButton) -> bool {
        self.just_released.contains(&button)
    }

    pub fn press(&mut self, button: MouseButton) {
        if self.pressed.insert(button) {
            self.just_pressed.insert(button);
        }
    }

    pub fn release(&mut self, button: MouseButton) {
        if self.pressed.remove(&button) {
            self.just_released.insert(button);
        }
    }

    pub fn set_position(&mut self, x: f32, y: f32) {
        let new_pos = glam::Vec2::new(x, y);
        self.delta = new_pos - self.position;
        self.position = new_pos;
    }

    pub fn set_scroll(&mut self, delta: f32) {
        self.scroll_delta = delta;
    }

    pub fn end_frame(&mut self) {
        self.just_pressed.clear();
        self.just_released.clear();
        self.delta = glam::Vec2::ZERO;
        self.scroll_delta = 0.0;
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_press_sets_is_pressed_and_just_pressed() {
        let mut mouse = MouseState::new();
        mouse.press(MouseButton::Left);

        assert!(mouse.is_pressed(MouseButton::Left));
        assert!(mouse.just_pressed(MouseButton::Left));
        assert!(!mouse.just_released(MouseButton::Left));
    }

    #[test]
    fn test_release_clears_is_pressed_sets_just_released() {
        let mut mouse = MouseState::new();
        mouse.press(MouseButton::Right);
        mouse.release(MouseButton::Right);

        assert!(!mouse.is_pressed(MouseButton::Right));
        assert!(mouse.just_released(MouseButton::Right));
    }

    #[test]
    fn test_set_position_computes_delta() {
        let mut mouse = MouseState::new();
        mouse.set_position(100.0, 200.0);

        assert_eq!(mouse.position, glam::Vec2::new(100.0, 200.0));
        assert_eq!(mouse.delta, glam::Vec2::new(100.0, 200.0));

        mouse.set_position(150.0, 250.0);

        assert_eq!(mouse.position, glam::Vec2::new(150.0, 250.0));
        assert_eq!(mouse.delta, glam::Vec2::new(50.0, 50.0));
    }

    #[test]
    fn test_set_scroll() {
        let mut mouse = MouseState::new();
        mouse.set_scroll(3.0);

        assert_eq!(mouse.scroll_delta, 3.0);
    }

    #[test]
    fn test_end_frame_resets_delta_and_scroll() {
        let mut mouse = MouseState::new();
        mouse.set_position(100.0, 200.0);
        mouse.set_scroll(5.0);
        mouse.press(MouseButton::Left);
        mouse.end_frame();

        assert_eq!(mouse.delta, glam::Vec2::ZERO);
        assert_eq!(mouse.scroll_delta, 0.0);
        assert!(mouse.is_pressed(MouseButton::Left));
        assert!(!mouse.just_pressed(MouseButton::Left));
    }

    #[test]
    fn test_end_frame_clears_just_released() {
        let mut mouse = MouseState::new();
        mouse.press(MouseButton::Middle);
        mouse.release(MouseButton::Middle);
        mouse.end_frame();

        assert!(!mouse.just_released(MouseButton::Middle));
    }

    #[test]
    fn test_double_press_does_not_duplicate() {
        let mut mouse = MouseState::new();
        mouse.press(MouseButton::Left);
        mouse.press(MouseButton::Left);

        assert!(mouse.is_pressed(MouseButton::Left));
        assert!(mouse.just_pressed(MouseButton::Left));
    }

    #[test]
    fn test_release_without_press_is_noop() {
        let mut mouse = MouseState::new();
        mouse.release(MouseButton::Right);

        assert!(!mouse.is_pressed(MouseButton::Right));
        assert!(!mouse.just_released(MouseButton::Right));
    }

    #[test]
    fn test_new_state_defaults() {
        let mouse = MouseState::new();

        assert_eq!(mouse.position, glam::Vec2::ZERO);
        assert_eq!(mouse.delta, glam::Vec2::ZERO);
        assert_eq!(mouse.scroll_delta, 0.0);
        assert!(!mouse.is_pressed(MouseButton::Left));
    }
}
