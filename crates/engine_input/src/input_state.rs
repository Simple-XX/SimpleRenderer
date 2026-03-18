// 聚合输入状态

use crate::keyboard::KeyboardState;
use crate::mouse::MouseState;

#[derive(Debug, Default, Clone)]
pub struct InputState {
    pub keyboard: KeyboardState,
    pub mouse: MouseState,
}

impl InputState {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn end_frame(&mut self) {
        self.keyboard.end_frame();
        self.mouse.end_frame();
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::keyboard::KeyCode;
    use crate::mouse::MouseButton;

    #[test]
    fn test_end_frame_delegates_to_keyboard_and_mouse() {
        let mut input = InputState::new();

        input.keyboard.press(KeyCode::W);
        input.mouse.press(MouseButton::Left);
        input.mouse.set_position(10.0, 20.0);
        input.mouse.set_scroll(1.0);

        assert!(input.keyboard.just_pressed(KeyCode::W));
        assert!(input.mouse.just_pressed(MouseButton::Left));

        input.end_frame();

        assert!(input.keyboard.is_pressed(KeyCode::W));
        assert!(!input.keyboard.just_pressed(KeyCode::W));
        assert!(input.mouse.is_pressed(MouseButton::Left));
        assert!(!input.mouse.just_pressed(MouseButton::Left));
        assert_eq!(input.mouse.delta, glam::Vec2::ZERO);
        assert_eq!(input.mouse.scroll_delta, 0.0);
    }

    #[test]
    fn test_new_input_state_is_default() {
        let input = InputState::new();

        assert!(!input.keyboard.is_pressed(KeyCode::A));
        assert!(!input.mouse.is_pressed(MouseButton::Left));
        assert_eq!(input.mouse.position, glam::Vec2::ZERO);
    }
}
