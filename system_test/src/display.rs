use minifb::{Key, MouseButton, MouseMode, Window, WindowOptions};
use simple_renderer::RenderingMode;

use crate::camera::{Camera, CameraMovement};

/// 窗口显示与输入处理
pub struct Display {
    /// minifb 窗口实例
    window: Window,
    /// 窗口宽度（像素）
    width: usize,
    /// 窗口高度（像素）
    height: usize,
    /// 格式转换缓冲区（内部 RGBA → minifb 0x00RRGGBB）
    minifb_buffer: Vec<u32>,
    /// 上一帧鼠标 X 坐标
    last_mouse_x: f32,
    /// 上一帧鼠标 Y 坐标
    last_mouse_y: f32,
    /// 鼠标右键是否处于拖拽状态（用于避免首次按下时的视角跳跃）
    right_dragging: bool,
}

impl Display {
    /// 创建窗口并初始化显示
    pub fn new(width: usize, height: usize) -> Self {
        let window = Window::new(
            "SimpleRenderer (Rust)",
            width,
            height,
            WindowOptions::default(),
        )
        .expect("创建窗口失败");

        Self {
            window,
            width,
            height,
            minifb_buffer: vec![0u32; width * height],
            last_mouse_x: width as f32 / 2.0,
            last_mouse_y: height as f32 / 2.0,
            right_dragging: false,
        }
    }

    /// 检查窗口是否仍然打开（Esc 或 Q 关闭）
    pub fn is_open(&self) -> bool {
        self.window.is_open()
            && !self.window.is_key_down(Key::Escape)
            && !self.window.is_key_down(Key::Q)
    }

    /// 将帧缓冲区内容显示到窗口
    ///
    /// 内部进行颜色格式转换：
    /// - 输入格式：R[0:7] G[8:15] B[16:23] A[24:31]
    /// - minifb 格式：0x00RRGGBB（R[16:23] G[8:15] B[0:7]）
    pub fn update(&mut self, buffer: &[u32]) {
        for (i, &pixel) in buffer.iter().enumerate() {
            let r = pixel & 0xFF;
            let g = (pixel >> 8) & 0xFF;
            let b = (pixel >> 16) & 0xFF;
            self.minifb_buffer[i] = (r << 16) | (g << 8) | b;
        }
        self.window
            .update_with_buffer(&self.minifb_buffer, self.width, self.height)
            .expect("更新窗口缓冲区失败");
    }

    /// 处理输入事件，返回可能切换的渲染模式
    ///
    /// # 操作说明
    /// - **WASD / 方向键**：前后左右移动
    /// - **空格 / 左Shift**：上升 / 下降
    /// - **鼠标右键拖拽**：旋转视角
    /// - **滚轮**：调整移动速度
    /// - **数字键 1-4**：切换渲染模式
    pub fn handle_input(&mut self, camera: &mut Camera, delta_time: f32) -> Option<RenderingMode> {
        // ===== 键盘移动 =====
        if self.window.is_key_down(Key::W) || self.window.is_key_down(Key::Up) {
            camera.process_keyboard(CameraMovement::Forward, delta_time);
        }
        if self.window.is_key_down(Key::S) || self.window.is_key_down(Key::Down) {
            camera.process_keyboard(CameraMovement::Backward, delta_time);
        }
        if self.window.is_key_down(Key::A) || self.window.is_key_down(Key::Left) {
            camera.process_keyboard(CameraMovement::Left, delta_time);
        }
        if self.window.is_key_down(Key::D) || self.window.is_key_down(Key::Right) {
            camera.process_keyboard(CameraMovement::Right, delta_time);
        }
        if self.window.is_key_down(Key::Space) {
            camera.process_keyboard(CameraMovement::Up, delta_time);
        }
        if self.window.is_key_down(Key::LeftShift) {
            camera.process_keyboard(CameraMovement::Down, delta_time);
        }

        // ===== 鼠标滚轮调整移动速度 =====
        if let Some((_, scroll_y)) = self.window.get_scroll_wheel() {
            if scroll_y > 0.0 {
                camera.adjust_speed(1.1);
            } else if scroll_y < 0.0 {
                camera.adjust_speed(0.9);
            }
        }

        // ===== 鼠标右键拖拽旋转视角 =====
        if let Some((mx, my)) = self.window.get_mouse_pos(MouseMode::Clamp) {
            if self.window.get_mouse_down(MouseButton::Right) {
                if self.right_dragging {
                    // 已经在拖拽中，计算位移并旋转
                    let dx = mx - self.last_mouse_x;
                    let dy = self.last_mouse_y - my; // Y 轴翻转：鼠标上移 = 视角上抬
                    camera.process_mouse(dx, dy);
                }
                // 记录当前位置，标记拖拽状态
                self.last_mouse_x = mx;
                self.last_mouse_y = my;
                self.right_dragging = true;
            } else {
                // 释放右键时重置拖拽状态，避免下次按下时产生跳跃
                self.right_dragging = false;
                self.last_mouse_x = mx;
                self.last_mouse_y = my;
            }
        }

        // ===== 数字键切换渲染模式 =====
        if self.window.is_key_down(Key::Key1) {
            return Some(RenderingMode::PerTriangle);
        }
        if self.window.is_key_down(Key::Key2) {
            return Some(RenderingMode::TileBased);
        }
        if self.window.is_key_down(Key::Key3) {
            return Some(RenderingMode::Deferred);
        }
        if self.window.is_key_down(Key::Key4) {
            return Some(RenderingMode::TileBasedDeferred);
        }

        None
    }

    /// 设置窗口标题
    pub fn set_title(&mut self, title: &str) {
        self.window.set_title(title);
    }
}
