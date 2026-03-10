use glam::{Mat4, Vec3};

#[allow(dead_code)]
pub struct Camera {
    position: Vec3,
    front: Vec3,
    up: Vec3,
}

#[allow(dead_code)]
impl Camera {
    pub fn new(position: Vec3) -> Self {
        Self {
            position,
            front: Vec3::new(0.0, 0.0, -1.0),
            up: Vec3::new(0.0, 1.0, 0.0),
        }
    }

    pub fn position(&self) -> Vec3 {
        self.position
    }

    pub fn view_matrix(&self) -> Mat4 {
        Mat4::look_at_rh(self.position, self.position + self.front, self.up)
    }

    pub fn projection_matrix(&self, fov_deg: f32, aspect: f32, near: f32, far: f32) -> Mat4 {
        Mat4::perspective_rh_gl(fov_deg.to_radians(), aspect, near, far)
    }

    pub fn move_forward(&mut self, distance: f32) {
        self.position += self.front * distance;
    }

    pub fn move_right(&mut self, distance: f32) {
        self.position += self.front.cross(self.up).normalize() * distance;
    }

    pub fn move_up(&mut self, distance: f32) {
        self.position += self.up * distance;
    }

    pub fn rotate(&mut self, yaw: f32, pitch: f32) {
        let yaw_rad = yaw.to_radians();
        let pitch_rad = pitch.to_radians();
        self.front = Vec3::new(
            yaw_rad.cos() * pitch_rad.cos(),
            pitch_rad.sin(),
            yaw_rad.sin() * pitch_rad.cos(),
        )
        .normalize();
    }
}
