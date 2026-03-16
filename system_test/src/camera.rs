use glam::{Mat4, Vec3};

/// 相机移动方向
pub enum CameraMovement {
    /// 沿观察方向前进
    Forward,
    /// 沿观察方向后退
    Backward,
    /// 向左平移
    Left,
    /// 向右平移
    Right,
    /// 沿世界上方向上升
    Up,
    /// 沿世界上方向下降
    Down,
}

/// 自由相机（FPS 风格）
///
/// 使用欧拉角控制朝向，支持：
/// - WASD / 方向键：前后左右移动
/// - 空格 / 左 Shift：上升 / 下降
/// - 鼠标右键拖拽：旋转视角
/// - 滚轮：调整移动速度
pub struct Camera {
    /// 相机世界坐标
    position: Vec3,
    /// 观察方向（单位向量）
    front: Vec3,
    /// 相机局部上方向（单位向量）
    up: Vec3,
    /// 相机局部右方向（单位向量）
    right: Vec3,
    /// 世界上方向（固定为 Y 轴正方向）
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
    /// 默认偏航角（面朝 -Z 方向）
    const DEFAULT_YAW: f32 = -90.0;
    /// 默认俯仰角（水平）
    const DEFAULT_PITCH: f32 = 0.0;

    /// 在指定位置创建自由相机，默认面朝 -Z 方向
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

    /// 获取相机世界坐标
    pub fn position(&self) -> Vec3 {
        self.position
    }

    #[allow(dead_code)]
    pub fn movement_speed(&self) -> f32 {
        self.movement_speed
    }

    /// 调整移动速度（乘以倍率），限制在 0.1 ~ 50.0 之间
    pub fn adjust_speed(&mut self, factor: f32) {
        self.movement_speed = (self.movement_speed * factor).clamp(0.1, 50.0);
    }

    /// 计算观察矩阵（右手坐标系）
    pub fn view_matrix(&self) -> Mat4 {
        Mat4::look_at_rh(self.position, self.position + self.front, self.up)
    }

    /// 计算透视投影矩阵（右手坐标系，OpenGL 深度范围）
    pub fn projection_matrix(&self, fov_deg: f32, aspect: f32, near: f32, far: f32) -> Mat4 {
        Mat4::perspective_rh_gl(fov_deg.to_radians(), aspect, near, far)
    }

    /// 处理键盘移动输入
    ///
    /// `delta_time` 为帧间隔时间（秒），保证移动速度与帧率无关
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
    /// `x_offset` 和 `y_offset` 为鼠标位移量（像素），
    /// 内部会乘以灵敏度系数后更新欧拉角
    pub fn process_mouse(&mut self, x_offset: f32, y_offset: f32) {
        self.yaw += x_offset * self.mouse_sensitivity;
        self.pitch = (self.pitch + y_offset * self.mouse_sensitivity).clamp(-89.0, 89.0);
        self.update_vectors();
    }

    /// 根据当前欧拉角重新计算相机的方向向量（前、右、上）
    fn update_vectors(&mut self) {
        let yaw_rad = self.yaw.to_radians();
        let pitch_rad = self.pitch.to_radians();

        // 从偏航角和俯仰角计算前方向
        self.front = Vec3::new(
            yaw_rad.cos() * pitch_rad.cos(),
            pitch_rad.sin(),
            yaw_rad.sin() * pitch_rad.cos(),
        )
        .normalize();

        // 右方向 = 前方向 × 世界上方向（归一化）
        self.right = self.front.cross(self.world_up).normalize();
        // 上方向 = 右方向 × 前方向（归一化）
        self.up = self.right.cross(self.front).normalize();
    }
}
