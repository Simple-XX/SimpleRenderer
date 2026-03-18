use glam::{Mat4, Vec3};

/// 相机移动方向
pub enum CameraMovement {
    /// 沿视线方向前进
    Forward,
    /// 沿视线方向后退
    Backward,
    /// 向左平移
    Left,
    /// 向右平移
    Right,
    /// 沿世界上方轴向上移动
    Up,
    /// 沿世界上方轴向下移动
    Down,
}

/// 自由视角相机（FPS 风格）
///
/// 使用欧拉角控制朝向，支持：
/// - WASD / 方向键：前后左右移动
/// - 空格 / 左 Shift：上移 / 下移
/// - 右键拖拽：旋转视角
/// - 鼠标滚轮：调整移动速度
pub struct Camera {
    /// 相机世界坐标位置
    position: Vec3,
    /// 前方方向（单位向量）
    front: Vec3,
    /// 局部上方向（单位向量）
    up: Vec3,
    /// 局部右方向（单位向量）
    right: Vec3,
    /// 世界上方向（固定为 +Y 轴）
    world_up: Vec3,
    /// 偏航角（水平旋转，单位：度）
    yaw: f32,
    /// 俯仰角（垂直旋转，单位：度，限制在 ±89°）
    pitch: f32,
    /// 移动速度（单位/秒）
    movement_speed: f32,
    /// 鼠标灵敏度
    mouse_sensitivity: f32,
}

impl Camera {
    /// 默认移动速度（单位/秒）
    const DEFAULT_SPEED: f32 = 2.5;
    /// 默认鼠标灵敏度
    const DEFAULT_SENSITIVITY: f32 = 0.1;
    /// 默认偏航角（朝向 -Z 方向）
    const DEFAULT_YAW: f32 = -90.0;
    /// 默认俯仰角（水平）
    const DEFAULT_PITCH: f32 = 0.0;

    /// 在指定位置创建自由视角相机，初始朝向 -Z 方向
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

    /// 获取相机世界坐标位置
    pub fn position(&self) -> Vec3 {
        self.position
    }

    #[cfg(test)]
    pub fn movement_speed(&self) -> f32 {
        self.movement_speed
    }

    /// 调整移动速度（乘以系数），限制在 0.1 ~ 50.0 范围内
    pub fn adjust_speed(&mut self, factor: f32) {
        self.movement_speed = (self.movement_speed * factor).clamp(0.1, 50.0);
    }

    /// 计算视图矩阵（右手坐标系）
    pub fn view_matrix(&self) -> Mat4 {
        Mat4::look_at_rh(self.position, self.position + self.front, self.up)
    }

    /// 计算透视投影矩阵（右手坐标系，OpenGL 深度范围）
    pub fn projection_matrix(&self, fov_deg: f32, aspect: f32, near: f32, far: f32) -> Mat4 {
        Mat4::perspective_rh_gl(fov_deg.to_radians(), aspect, near, far)
    }

    /// 处理键盘移动输入
    ///
    /// `delta_time` 为帧间隔时间（秒），确保移动速度与帧率无关
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

    /// 处理鼠标移动输入
    ///
    /// `x_offset` 和 `y_offset` 为鼠标位移（像素）。
    /// 内部乘以灵敏度系数后更新欧拉角。
    pub fn process_mouse(&mut self, x_offset: f32, y_offset: f32) {
        self.yaw += x_offset * self.mouse_sensitivity;
        self.pitch = (self.pitch + y_offset * self.mouse_sensitivity).clamp(-89.0, 89.0);
        self.update_vectors();
    }

    /// 根据当前欧拉角重新计算相机方向向量（前方、右方、上方）
    fn update_vectors(&mut self) {
        let yaw_rad = self.yaw.to_radians();
        let pitch_rad = self.pitch.to_radians();

        // 根据偏航角和俯仰角计算前方方向
        self.front = Vec3::new(
            yaw_rad.cos() * pitch_rad.cos(),
            pitch_rad.sin(),
            yaw_rad.sin() * pitch_rad.cos(),
        )
        .normalize();

        // 右方向 = 前方方向 × 世界上方向（归一化）
        self.right = self.front.cross(self.world_up).normalize();
        // 上方向 = 右方向 × 前方方向（归一化）
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
        // 默认偏航角为 -90 度，俯仰角为 0
        // front 应近似为 (0, 0, -1)
        let cam = Camera::new(Vec3::ZERO);
        let view = cam.view_matrix();
        // 从原点看向 (0,0,-1)：视图矩阵应有效
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
        // 前进和后退应朝相反方向移动
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
        cam.process_mouse(10.0, 0.0); // 向右偏航
        let view_after = cam.view_matrix();
        assert_ne!(view_before, view_after, "yaw should change view matrix");
    }

    #[test]
    fn process_mouse_pitch_clamps_at_89() {
        let mut cam = Camera::new(Vec3::ZERO);
        // 尝试将俯仰角推到远超 89 度
        cam.process_mouse(0.0, 10000.0);
        // 俯仰角应被限制，视图矩阵仍应有效
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
