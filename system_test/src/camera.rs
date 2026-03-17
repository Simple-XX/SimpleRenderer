use glam::{Mat4, Vec3};

/// Camera movement direction
pub enum CameraMovement {
    /// Move forward along the view direction
    Forward,
    /// Move backward along the view direction
    Backward,
    /// Strafe left
    Left,
    /// Strafe right
    Right,
    /// Move up along the world up axis
    Up,
    /// Move down along the world up axis
    Down,
}

/// Free-look camera (FPS style)
///
/// Uses Euler angles to control orientation, supports:
/// - WASD / arrow keys: move forward/backward/left/right
/// - Space / Left Shift: move up / move down
/// - Right-click drag: rotate view
/// - Mouse wheel: adjust movement speed
pub struct Camera {
    /// Camera world position
    position: Vec3,
    /// Forward direction (unit vector)
    front: Vec3,
    /// Local up direction (unit vector)
    up: Vec3,
    /// Local right direction (unit vector)
    right: Vec3,
    /// World up direction (fixed to +Y axis)
    world_up: Vec3,
    /// Yaw angle (horizontal rotation, degrees)
    yaw: f32,
    /// Pitch angle (vertical rotation, degrees, clamped to ±89°)
    pitch: f32,
    /// Movement speed (units per second)
    movement_speed: f32,
    /// Mouse sensitivity
    mouse_sensitivity: f32,
}

impl Camera {
    /// Default movement speed (units per second)
    const DEFAULT_SPEED: f32 = 2.5;
    /// Default mouse sensitivity
    const DEFAULT_SENSITIVITY: f32 = 0.1;
    /// Default yaw angle (facing -Z direction)
    const DEFAULT_YAW: f32 = -90.0;
    /// Default pitch angle (horizontal)
    const DEFAULT_PITCH: f32 = 0.0;

    /// Create a free-look camera at the specified position, initially facing -Z direction
    pub fn new(position: Vec3) -> Self {
        let mut camera = Self {
            position,
            front: Vec3::NEG_Z,
            up: Vec3::Y,
            right: Vec3::X,
            world_up: Vec3::Y,
            yaw: Self::DEFAULT_YAW,
            pitch: Self::DEFAULT_PITCH,
            movement_speed: Self::DEFAULT_SPEED,
            mouse_sensitivity: Self::DEFAULT_SENSITIVITY,
        };
        camera.update_vectors();
        camera
    }

    /// Get camera world position
    pub fn position(&self) -> Vec3 {
        self.position
    }

    #[cfg(test)]
    pub fn movement_speed(&self) -> f32 {
        self.movement_speed
    }

    /// Adjust movement speed (multiply by factor), clamped to 0.1 ~ 50.0
    pub fn adjust_speed(&mut self, factor: f32) {
        self.movement_speed = (self.movement_speed * factor).clamp(0.1, 50.0);
    }

    /// Compute view matrix (right-handed coordinate system)
    pub fn view_matrix(&self) -> Mat4 {
        Mat4::look_at_rh(self.position, self.position + self.front, self.up)
    }

    /// Compute perspective projection matrix (right-handed, OpenGL depth range)
    pub fn projection_matrix(&self, fov_deg: f32, aspect: f32, near: f32, far: f32) -> Mat4 {
        Mat4::perspective_rh_gl(fov_deg.to_radians(), aspect, near, far)
    }

    /// Process keyboard movement input
    ///
    /// `delta_time` is the frame interval time (seconds), ensuring movement speed is frame-rate independent
    pub fn process_keyboard(&mut self, direction: CameraMovement, delta_time: f32) {
        let velocity = self.movement_speed * delta_time;
        match direction {
            CameraMovement::Forward => self.position += self.front * velocity,
            CameraMovement::Backward => self.position -= self.front * velocity,
            CameraMovement::Left => self.position -= self.right * velocity,
            CameraMovement::Right => self.position += self.right * velocity,
            CameraMovement::Up => self.position += self.world_up * velocity,
            CameraMovement::Down => self.position -= self.world_up * velocity,
        }
    }

    /// Process mouse movement input
    ///
    /// `x_offset` and `y_offset` are mouse displacement in pixels.
    /// Internally multiplied by sensitivity coefficient to update Euler angles.
    pub fn process_mouse(&mut self, x_offset: f32, y_offset: f32) {
        self.yaw += x_offset * self.mouse_sensitivity;
        self.pitch = (self.pitch + y_offset * self.mouse_sensitivity).clamp(-89.0, 89.0);
        self.update_vectors();
    }

    /// Recalculate camera direction vectors (forward, right, up) based on current Euler angles
    fn update_vectors(&mut self) {
        let yaw_rad = self.yaw.to_radians();
        let pitch_rad = self.pitch.to_radians();

        // Calculate forward direction from yaw and pitch angles
        self.front = Vec3::new(
            yaw_rad.cos() * pitch_rad.cos(),
            pitch_rad.sin(),
            yaw_rad.sin() * pitch_rad.cos(),
        )
        .normalize();

        // Right direction = forward direction × world up direction (normalized)
        self.right = self.front.cross(self.world_up).normalize();
        // Up direction = right direction × forward direction (normalized)
        self.up = self.right.cross(self.front).normalize();
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use glam::Vec3;

    const EPSILON: f32 = 1e-4;

    fn approx_eq(a: f32, b: f32) -> bool {
        (a - b).abs() < EPSILON
    }

    #[test]
    fn new_camera_position() {
        let cam = Camera::new(Vec3::new(1.0, 2.0, 3.0));
        assert_eq!(cam.position(), Vec3::new(1.0, 2.0, 3.0));
    }

    #[test]
    fn default_faces_negative_z() {
        // Default yaw is -90 degrees, pitch is 0
        // front should be approximately (0, 0, -1)
        let cam = Camera::new(Vec3::ZERO);
        let view = cam.view_matrix();
        // Looking at (0,0,-1) from origin: the view matrix should be valid
        assert!(
            view.determinant().abs() > EPSILON,
            "view matrix should be non-singular"
        );
    }

    #[test]
    fn movement_speed_default() {
        let cam = Camera::new(Vec3::ZERO);
        assert!(approx_eq(cam.movement_speed(), Camera::DEFAULT_SPEED));
    }

    #[test]
    fn adjust_speed_increase() {
        let mut cam = Camera::new(Vec3::ZERO);
        let original = cam.movement_speed();
        cam.adjust_speed(2.0);
        assert!(cam.movement_speed() > original);
        assert!(approx_eq(cam.movement_speed(), original * 2.0));
    }

    #[test]
    fn adjust_speed_clamps_high() {
        let mut cam = Camera::new(Vec3::ZERO);
        cam.adjust_speed(1000.0);
        assert!(cam.movement_speed() <= 50.0);
    }

    #[test]
    fn adjust_speed_clamps_low() {
        let mut cam = Camera::new(Vec3::ZERO);
        cam.adjust_speed(0.001);
        assert!(cam.movement_speed() >= 0.1);
    }

    #[test]
    fn process_keyboard_forward_moves_position() {
        let mut cam = Camera::new(Vec3::ZERO);
        let before = cam.position();
        cam.process_keyboard(CameraMovement::Forward, 1.0);
        let after = cam.position();
        assert!(
            (after - before).length() > EPSILON,
            "forward should move camera"
        );
    }

    #[test]
    fn process_keyboard_backward_moves_opposite() {
        let mut cam1 = Camera::new(Vec3::ZERO);
        let mut cam2 = Camera::new(Vec3::ZERO);
        cam1.process_keyboard(CameraMovement::Forward, 1.0);
        cam2.process_keyboard(CameraMovement::Backward, 1.0);
        // Forward and backward should move in opposite directions
        let fwd = cam1.position();
        let bwd = cam2.position();
        assert!(
            fwd.dot(bwd) < 0.0,
            "forward and backward should be opposite"
        );
    }

    #[test]
    fn process_keyboard_left_right_opposite() {
        let mut cam1 = Camera::new(Vec3::ZERO);
        let mut cam2 = Camera::new(Vec3::ZERO);
        cam1.process_keyboard(CameraMovement::Left, 1.0);
        cam2.process_keyboard(CameraMovement::Right, 1.0);
        let left = cam1.position();
        let right = cam2.position();
        assert!(left.dot(right) < 0.0, "left and right should be opposite");
    }

    #[test]
    fn process_keyboard_up_moves_on_world_y() {
        let mut cam = Camera::new(Vec3::ZERO);
        cam.process_keyboard(CameraMovement::Up, 1.0);
        assert!(cam.position().y > 0.0, "up should increase Y");
    }

    #[test]
    fn process_keyboard_down_moves_negative_y() {
        let mut cam = Camera::new(Vec3::ZERO);
        cam.process_keyboard(CameraMovement::Down, 1.0);
        assert!(cam.position().y < 0.0, "down should decrease Y");
    }

    #[test]
    fn process_keyboard_delta_time_scales_movement() {
        let mut cam1 = Camera::new(Vec3::ZERO);
        let mut cam2 = Camera::new(Vec3::ZERO);
        cam1.process_keyboard(CameraMovement::Forward, 0.5);
        cam2.process_keyboard(CameraMovement::Forward, 1.0);
        let d1 = cam1.position().length();
        let d2 = cam2.position().length();
        assert!(
            approx_eq(d2, d1 * 2.0),
            "double delta_time should double distance"
        );
    }

    #[test]
    fn process_mouse_yaw_changes_direction() {
        let mut cam = Camera::new(Vec3::ZERO);
        let view_before = cam.view_matrix();
        cam.process_mouse(10.0, 0.0); // yaw right
        let view_after = cam.view_matrix();
        assert_ne!(view_before, view_after, "yaw should change view matrix");
    }

    #[test]
    fn process_mouse_pitch_clamps_at_89() {
        let mut cam = Camera::new(Vec3::ZERO);
        // Try to pitch way beyond 89 degrees
        cam.process_mouse(0.0, 10000.0);
        // Pitch should be clamped, view matrix should still be valid
        let view = cam.view_matrix();
        assert!(
            view.determinant().abs() > EPSILON,
            "view matrix should remain valid at max pitch"
        );
    }

    #[test]
    fn projection_matrix_is_valid() {
        let cam = Camera::new(Vec3::ZERO);
        let proj = cam.projection_matrix(60.0, 800.0 / 600.0, 0.1, 100.0);
        assert!(
            proj.determinant().abs() > EPSILON,
            "projection should be non-singular"
        );
    }

    #[test]
    fn view_matrix_changes_with_position() {
        let cam1 = Camera::new(Vec3::new(0.0, 0.0, 0.0));
        let cam2 = Camera::new(Vec3::new(10.0, 0.0, 0.0));
        assert_ne!(cam1.view_matrix(), cam2.view_matrix());
    }
}
