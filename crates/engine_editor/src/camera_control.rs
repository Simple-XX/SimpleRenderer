// Copyright The SimpleGameEngine Contributors


//! 编辑器相机控制器（FPS 风格）
//!
//! 从 system_test/src/camera.rs 迁移而来，适配 egui 输入系统。
//! 支持右键拖拽旋转视角、WASD 键移动。

use glam::{Mat4, Vec3};

/// 编辑器自由视角相机
pub struct EditorCamera {
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
    pub movement_speed: f32,
    /// 鼠标灵敏度
    pub mouse_sensitivity: f32,
    /// 视场角（度）
    pub fov: f32,
    /// 近裁剪面
    pub near: f32,
    /// 远裁剪面
    pub far: f32,
}

impl Default for EditorCamera {
    fn default() -> Self {
        Self::new(Vec3::new(0.0, 0.0, 3.0))
    }
}

impl EditorCamera {
    /// 默认移动速度（单位/秒）
    const DEFAULT_SPEED: f32 = 2.5;
    /// 默认鼠标灵敏度
    const DEFAULT_SENSITIVITY: f32 = 0.1;
    /// 默认偏航角（朝向 -Z 方向）
    const DEFAULT_YAW: f32 = -90.0;
    /// 默认俯仰角（水平）
    const DEFAULT_PITCH: f32 = 0.0;

    /// 在指定位置创建编辑器相机
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
            fov: 60.0,
            near: 0.1,
            far: 100.0,
        };
        camera.update_vectors();
        camera
    }

    /// 获取相机世界坐标位置
    pub fn position(&self) -> Vec3 {
        self.position
    }

    /// 计算视图矩阵（右手坐标系）
    pub fn view_matrix(&self) -> Mat4 {
        Mat4::look_at_rh(self.position, self.position + self.front, self.up)
    }

    /// 计算透视投影矩阵
    pub fn projection_matrix(&self, aspect: f32) -> Mat4 {
        Mat4::perspective_rh_gl(self.fov.to_radians(), aspect, self.near, self.far)
    }

    /// 处理鼠标旋转输入（右键拖拽时调用）
    pub fn process_mouse(&mut self, delta_x: f32, delta_y: f32) {
        self.yaw += delta_x * self.mouse_sensitivity;
        self.pitch = (self.pitch - delta_y * self.mouse_sensitivity).clamp(-89.0, 89.0);
        self.update_vectors();
    }

    /// 向前移动
    pub fn move_forward(&mut self, delta_time: f32) {
        self.position += self.front * self.movement_speed * delta_time;
    }

    /// 向后移动
    pub fn move_backward(&mut self, delta_time: f32) {
        self.position -= self.front * self.movement_speed * delta_time;
    }

    /// 向左移动
    pub fn move_left(&mut self, delta_time: f32) {
        self.position -= self.right * self.movement_speed * delta_time;
    }

    /// 向右移动
    pub fn move_right(&mut self, delta_time: f32) {
        self.position += self.right * self.movement_speed * delta_time;
    }

    /// 向上移动
    pub fn move_up(&mut self, delta_time: f32) {
        self.position += self.world_up * self.movement_speed * delta_time;
    }

    /// 向下移动
    pub fn move_down(&mut self, delta_time: f32) {
        self.position -= self.world_up * self.movement_speed * delta_time;
    }

    /// 根据当前欧拉角重新计算相机方向向量
    fn update_vectors(&mut self) {
        let yaw_rad = self.yaw.to_radians();
        let pitch_rad = self.pitch.to_radians();

        self.front = Vec3::new(
            yaw_rad.cos() * pitch_rad.cos(),
            pitch_rad.sin(),
            yaw_rad.sin() * pitch_rad.cos(),
        )
        .normalize();

        self.right = self.front.cross(self.world_up).normalize();
        self.up = self.right.cross(self.front).normalize();
    }
}
