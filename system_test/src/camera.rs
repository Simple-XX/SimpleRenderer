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

    #[allow(dead_code)]
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
