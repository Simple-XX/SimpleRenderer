// 键盘输入状态管理

use std::collections::HashSet;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum KeyCode {
    W,
    A,
    S,
    D,
    Q,
    E,
    Space,
    LShift,
    LControl,
    Escape,
    Tab,
    Up,
    Down,
    Left,
    Right,
    Key1,
    Key2,
    Key3,
    Key4,
    Key5,
    Key6,
    Key7,
    Key8,
    Key9,
    Key0,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    Enter,
    Backspace,
    Delete,
    Home,
    End,
    PageUp,
    PageDown,
    B,
    C,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    R,
    T,
    U,
    V,
    X,
    Y,
    Z,
}

#[derive(Debug, Default, Clone)]
pub struct KeyboardState {
    pressed: HashSet<KeyCode>,
    just_pressed: HashSet<KeyCode>,
    just_released: HashSet<KeyCode>,
}

impl KeyboardState {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn is_pressed(&self, key: KeyCode) -> bool {
        self.pressed.contains(&key)
    }

    pub fn just_pressed(&self, key: KeyCode) -> bool {
        self.just_pressed.contains(&key)
    }

    pub fn just_released(&self, key: KeyCode) -> bool {
        self.just_released.contains(&key)
    }

    pub fn press(&mut self, key: KeyCode) {
        if self.pressed.insert(key) {
            self.just_pressed.insert(key);
        }
    }

    pub fn release(&mut self, key: KeyCode) {
        if self.pressed.remove(&key) {
            self.just_released.insert(key);
        }
    }

    pub fn end_frame(&mut self) {
        self.just_pressed.clear();
        self.just_released.clear();
    }

    pub fn pressed_keys(&self) -> &HashSet<KeyCode> {
        &self.pressed
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_press_sets_is_pressed_and_just_pressed() {
        let mut kb = KeyboardState::new();
        kb.press(KeyCode::W);

        assert!(kb.is_pressed(KeyCode::W));
        assert!(kb.just_pressed(KeyCode::W));
        assert!(!kb.just_released(KeyCode::W));
    }

    #[test]
    fn test_release_clears_is_pressed_sets_just_released() {
        let mut kb = KeyboardState::new();
        kb.press(KeyCode::A);
        kb.release(KeyCode::A);

        assert!(!kb.is_pressed(KeyCode::A));
        assert!(kb.just_released(KeyCode::A));
    }

    #[test]
    fn test_end_frame_clears_just_states_keeps_pressed() {
        let mut kb = KeyboardState::new();
        kb.press(KeyCode::S);
        kb.end_frame();

        assert!(kb.is_pressed(KeyCode::S));
        assert!(!kb.just_pressed(KeyCode::S));
    }

    #[test]
    fn test_end_frame_clears_just_released() {
        let mut kb = KeyboardState::new();
        kb.press(KeyCode::D);
        kb.release(KeyCode::D);
        kb.end_frame();

        assert!(!kb.just_released(KeyCode::D));
    }

    #[test]
    fn test_double_press_does_not_duplicate_just_pressed() {
        let mut kb = KeyboardState::new();
        kb.press(KeyCode::Space);
        kb.press(KeyCode::Space);

        assert!(kb.just_pressed(KeyCode::Space));
        assert!(kb.is_pressed(KeyCode::Space));

        let count = kb
            .pressed_keys()
            .iter()
            .filter(|&&k| k == KeyCode::Space)
            .count();
        assert_eq!(count, 1);
    }

    #[test]
    fn test_release_without_press_is_noop() {
        let mut kb = KeyboardState::new();
        kb.release(KeyCode::Escape);

        assert!(!kb.is_pressed(KeyCode::Escape));
        assert!(!kb.just_released(KeyCode::Escape));
    }

    #[test]
    fn test_pressed_keys_returns_all_pressed() {
        let mut kb = KeyboardState::new();
        kb.press(KeyCode::W);
        kb.press(KeyCode::A);
        kb.press(KeyCode::S);

        let keys = kb.pressed_keys();
        assert_eq!(keys.len(), 3);
        assert!(keys.contains(&KeyCode::W));
        assert!(keys.contains(&KeyCode::A));
        assert!(keys.contains(&KeyCode::S));
    }

    #[test]
    fn test_multiple_keys_independent() {
        let mut kb = KeyboardState::new();
        kb.press(KeyCode::W);
        kb.press(KeyCode::A);
        kb.release(KeyCode::W);

        assert!(!kb.is_pressed(KeyCode::W));
        assert!(kb.is_pressed(KeyCode::A));
        assert!(kb.just_released(KeyCode::W));
        assert!(!kb.just_released(KeyCode::A));
    }

    #[test]
    fn test_new_state_is_empty() {
        let kb = KeyboardState::new();

        assert!(!kb.is_pressed(KeyCode::W));
        assert!(!kb.just_pressed(KeyCode::W));
        assert!(!kb.just_released(KeyCode::W));
        assert!(kb.pressed_keys().is_empty());
    }
}
