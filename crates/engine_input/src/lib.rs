// 统一输入状态管理

pub mod input_state;
pub mod keyboard;
pub mod mouse;

pub use input_state::InputState;
pub use keyboard::{KeyCode, KeyboardState};
pub use mouse::{MouseButton, MouseState};
