//! 编辑器相机控制器（多模式）
//!
//! 支持 FPS 飞行、环绕（Orbit）、平移（Pan）、推拉（Dolly）、聚焦（Focus）等交互模式。

use glam::{Mat4, Vec3};

/// 相机交互模式
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
#[allow(dead_code)]
pub enum CameraMode {
    /// 空闲（无操作）
    #[default]
    Idle,
    /// FPS 自由飞行
    Fly,
    /// 以 pivot 为中心环绕旋转
    Orbit,
    /// 垂直于视线方向平移
    Pan,
    /// 沿视线方向推拉
    Dolly,
}

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
    /// 当前交互模式
    #[allow(dead_code)]
    pub mode: CameraMode,
    /// 环绕模式的轴心点
    #[allow(dead_code)]
    pub orbit_pivot: Vec3,
    /// 飞行速度（可调 0.5–50.0）
    #[allow(dead_code)]
    fly_speed: f32,
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
            mode: CameraMode::default(),
            orbit_pivot: Vec3::ZERO,
            fly_speed: Self::DEFAULT_SPEED,
        };
        camera.update_vectors();
        camera
    }

    pub fn position(&self) -> Vec3 {
        self.position
    }

    pub fn set_position(&mut self, pos: Vec3) {
        self.position = pos;
        self.update_vectors();
    }

    pub fn set_rotation(&mut self, yaw: f32, pitch: f32) {
        self.yaw = yaw;
        self.pitch = pitch.clamp(-89.0, 89.0);
        self.update_vectors();
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

    /// 环绕模式：以 orbit_pivot 为轴心旋转相机
    #[allow(dead_code)]
    pub fn orbit(&mut self, dx: f32, dy: f32) {
        let radius = (self.position - self.orbit_pivot).length();
        self.yaw += dx * self.mouse_sensitivity;
        self.pitch = (self.pitch - dy * self.mouse_sensitivity).clamp(-89.0, 89.0);
        self.update_vectors();
        self.position = self.orbit_pivot - self.front * radius;
    }

    /// 平移模式：垂直于视线方向平移相机和 pivot
    #[allow(dead_code)]
    pub fn pan(&mut self, dx: f32, dy: f32) {
        let speed = 0.01;
        let offset = self.right * (-dx * speed) + self.up * (dy * speed);
        self.position += offset;
        self.orbit_pivot += offset;
    }

    /// 推拉模式：沿视线方向前后移动
    #[allow(dead_code)]
    pub fn dolly(&mut self, delta: f32) {
        let movement = self.front * delta * 0.05;
        self.position += movement;
    }

    /// 聚焦到目标位置，相机飞向目标
    #[allow(dead_code)]
    pub fn focus_on(&mut self, target: Vec3, size: f32) {
        let distance = (size * 2.5).max(1.0);
        let diff = self.position - target;
        let direction = if diff.length_squared() > 0.0 {
            diff.normalize()
        } else {
            Vec3::new(0.0, 0.0, 1.0)
        };
        self.position = target + direction * distance;
        self.look_at(target);
        self.orbit_pivot = target;
    }

    /// 让相机朝向目标
    #[allow(dead_code)]
    fn look_at(&mut self, target: Vec3) {
        let dir = (target - self.position).normalize();
        self.pitch = dir.y.asin().to_degrees();
        self.yaw = dir.z.atan2(dir.x).to_degrees();
        self.update_vectors();
    }

    /// 滚轮缩放
    #[allow(dead_code)]
    pub fn scroll_zoom(&mut self, delta: f32, fast: bool) {
        let speed = if fast { 5.0 } else { 1.0 };
        self.position += self.front * delta * speed * 0.5;
    }

    /// 获取飞行速度
    #[allow(dead_code)]
    pub fn fly_speed(&self) -> f32 {
        self.fly_speed
    }

    /// 调整飞行速度（限制在 0.5–50.0 范围内）
    #[allow(dead_code)]
    pub fn adjust_fly_speed(&mut self, delta: f32) {
        self.fly_speed = (self.fly_speed + delta * 0.5).clamp(0.5, 50.0);
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

#[cfg(test)]
mod tests {
    use super::*;

    fn approx_eq(a: Vec3, b: Vec3, eps: f32) -> bool {
        (a.x - b.x).abs() < eps && (a.y - b.y).abs() < eps && (a.z - b.z).abs() < eps
    }

    #[test]
    fn camera_mode_default_is_idle() {
        assert_eq!(CameraMode::default(), CameraMode::Idle);
    }

    #[test]
    fn new_camera_initializes_fields() {
        let cam = EditorCamera::new(Vec3::new(1.0, 2.0, 3.0));
        assert_eq!(cam.position(), Vec3::new(1.0, 2.0, 3.0));
        assert_eq!(cam.mode, CameraMode::Idle);
        assert_eq!(cam.orbit_pivot, Vec3::ZERO);
        assert!((cam.fly_speed() - 2.5).abs() < f32::EPSILON);
    }

    #[test]
    fn default_camera_position() {
        let cam = EditorCamera::default();
        assert_eq!(cam.position(), Vec3::new(0.0, 0.0, 3.0));
    }

    #[test]
    fn orbit_preserves_distance_to_pivot() {
        let mut cam = EditorCamera::new(Vec3::new(0.0, 0.0, 5.0));
        cam.orbit_pivot = Vec3::ZERO;
        let initial_dist = (cam.position() - cam.orbit_pivot).length();

        cam.orbit(100.0, 50.0);

        let final_dist = (cam.position() - cam.orbit_pivot).length();
        assert!((initial_dist - final_dist).abs() < 0.01);
    }

    #[test]
    fn orbit_updates_yaw_and_pitch() {
        let mut cam = EditorCamera::new(Vec3::new(0.0, 0.0, 5.0));
        cam.orbit_pivot = Vec3::ZERO;
        let old_yaw = cam.yaw;
        let old_pitch = cam.pitch;

        cam.orbit(10.0, 5.0);

        assert!((cam.yaw - (old_yaw + 10.0 * cam.mouse_sensitivity)).abs() < 0.001);
        let expected_pitch = (old_pitch - 5.0 * cam.mouse_sensitivity).clamp(-89.0, 89.0);
        assert!((cam.pitch - expected_pitch).abs() < 0.001);
    }

    #[test]
    fn pan_moves_camera_and_pivot_equally() {
        let mut cam = EditorCamera::default();
        let old_pos = cam.position();
        let old_pivot = cam.orbit_pivot;

        cam.pan(100.0, 50.0);

        let pos_delta = cam.position() - old_pos;
        let pivot_delta = cam.orbit_pivot - old_pivot;
        assert!(approx_eq(pos_delta, pivot_delta, 0.0001));
    }

    #[test]
    fn pan_moves_perpendicular_to_view() {
        let mut cam = EditorCamera::default();
        let old_pos = cam.position();

        cam.pan(100.0, 0.0);

        let delta = cam.position() - old_pos;
        // 平移应垂直于前方向：delta·front ≈ 0
        assert!(delta.dot(cam.front).abs() < 0.001);
    }

    #[test]
    fn dolly_moves_along_front() {
        let mut cam = EditorCamera::default();
        let old_pos = cam.position();
        let front = cam.front;

        cam.dolly(10.0);

        let delta = cam.position() - old_pos;
        let expected = front * 10.0 * 0.05;
        assert!(approx_eq(delta, expected, 0.0001));
    }

    #[test]
    fn focus_on_sets_pivot_and_distance() {
        let mut cam = EditorCamera::new(Vec3::new(10.0, 0.0, 0.0));
        let target = Vec3::new(1.0, 2.0, 3.0);

        cam.focus_on(target, 2.0);

        assert_eq!(cam.orbit_pivot, target);
        let dist = (cam.position() - target).length();
        // size=2.0 → distance = max(2.0*2.5, 1.0) = 5.0
        assert!((dist - 5.0).abs() < 0.01);
    }

    #[test]
    fn focus_on_small_size_clamps_distance() {
        let mut cam = EditorCamera::default();
        let target = Vec3::new(0.0, 0.0, 0.0);

        cam.focus_on(target, 0.1);

        let dist = (cam.position() - target).length();
        // size=0.1 → distance = max(0.25, 1.0) = 1.0
        assert!((dist - 1.0).abs() < 0.01);
    }

    #[test]
    fn focus_on_same_position_uses_fallback_direction() {
        let mut cam = EditorCamera::new(Vec3::ZERO);
        cam.focus_on(Vec3::ZERO, 1.0);

        let dist = cam.position().length();
        // distance = max(2.5, 1.0) = 2.5, fallback direction = +Z
        assert!((dist - 2.5).abs() < 0.01);
    }

    #[test]
    fn scroll_zoom_normal_speed() {
        let mut cam = EditorCamera::default();
        let old_pos = cam.position();
        let front = cam.front;

        cam.scroll_zoom(2.0, false);

        let expected = old_pos + front * 2.0 * 1.0 * 0.5;
        assert!(approx_eq(cam.position(), expected, 0.0001));
    }

    #[test]
    fn scroll_zoom_fast_speed() {
        let mut cam = EditorCamera::default();
        let old_pos = cam.position();
        let front = cam.front;

        cam.scroll_zoom(2.0, true);

        let expected = old_pos + front * 2.0 * 5.0 * 0.5;
        assert!(approx_eq(cam.position(), expected, 0.0001));
    }

    #[test]
    fn fly_speed_getter() {
        let cam = EditorCamera::default();
        assert!((cam.fly_speed() - 2.5).abs() < f32::EPSILON);
    }

    #[test]
    fn adjust_fly_speed_increases() {
        let mut cam = EditorCamera::default();
        cam.adjust_fly_speed(5.0);
        assert!((cam.fly_speed() - 5.0).abs() < f32::EPSILON);
    }

    #[test]
    fn adjust_fly_speed_clamps_low() {
        let mut cam = EditorCamera::default();
        cam.adjust_fly_speed(-100.0);
        assert!((cam.fly_speed() - 0.5).abs() < f32::EPSILON);
    }

    #[test]
    fn adjust_fly_speed_clamps_high() {
        let mut cam = EditorCamera::default();
        cam.adjust_fly_speed(200.0);
        assert!((cam.fly_speed() - 50.0).abs() < f32::EPSILON);
    }

    #[test]
    fn existing_movement_still_works() {
        let mut cam = EditorCamera::default();
        let old_pos = cam.position();

        cam.move_forward(1.0);
        assert_ne!(cam.position(), old_pos);

        let pos_after_fwd = cam.position();
        cam.move_backward(1.0);
        assert!(approx_eq(cam.position(), old_pos, 0.001));

        cam.set_position(old_pos);
        cam.move_left(1.0);
        assert_ne!(cam.position(), old_pos);

        cam.move_right(1.0);
        assert!(approx_eq(cam.position(), old_pos, 0.001));

        cam.move_up(1.0);
        assert!(cam.position().y > old_pos.y);

        cam.move_down(1.0);
        assert!(approx_eq(cam.position(), old_pos, 0.001));

        let _ = pos_after_fwd;
    }

    #[test]
    fn process_mouse_still_works() {
        let mut cam = EditorCamera::default();
        let old_yaw = cam.yaw;
        cam.process_mouse(10.0, 0.0);
        assert!((cam.yaw - (old_yaw + 10.0 * cam.mouse_sensitivity)).abs() < 0.001);
    }
}
