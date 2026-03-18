#![allow(dead_code)]
use std::time::{Duration, Instant};

/// Toast 通知级别
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[allow(dead_code)]
pub enum ToastLevel {
    /// 成功 — 绿色背景，3 秒消失
    Success,
    /// 错误 — 红色背景，5 秒消失
    Error,
    /// 信息 — 灰色背景，3 秒消失
    Info,
}

/// 单条 Toast 通知
pub struct Toast {
    pub message: String,
    pub level: ToastLevel,
    pub created: Instant,
    pub duration: Duration,
}

/// Toast 管理器，维护活跃的通知列表
#[derive(Default)]
pub struct ToastManager {
    toasts: Vec<Toast>,
}

#[allow(dead_code)]
impl ToastManager {
    /// 创建新的 Toast 管理器
    pub fn new() -> Self {
        Self { toasts: Vec::new() }
    }

    /// 添加成功通知
    pub fn success(&mut self, message: impl Into<String>) {
        self.add(message, ToastLevel::Success);
    }

    /// 添加错误通知
    pub fn error(&mut self, message: impl Into<String>) {
        self.add(message, ToastLevel::Error);
    }

    /// 添加信息通知
    pub fn info(&mut self, message: impl Into<String>) {
        self.add(message, ToastLevel::Info);
    }

    /// 内部方法：添加 Toast
    fn add(&mut self, message: impl Into<String>, level: ToastLevel) {
        let duration = match level {
            ToastLevel::Success => Duration::from_secs(3),
            ToastLevel::Error => Duration::from_secs(5),
            ToastLevel::Info => Duration::from_secs(3),
        };
        self.toasts.push(Toast {
            message: message.into(),
            level,
            created: Instant::now(),
            duration,
        });
    }

    /// 清除已过期的 Toast
    pub fn cleanup(&mut self) {
        self.toasts.retain(|t| t.created.elapsed() < t.duration);
    }

    /// 当前活跃的 Toast 数量
    pub fn count(&self) -> usize {
        self.toasts.len()
    }
}

/// 在指定区域的右下角绘制 Toast 通知
pub fn show(ctx: &egui::Context, manager: &mut ToastManager) {
    manager.cleanup();

    if manager.toasts.is_empty() {
        return;
    }

    egui::Area::new(egui::Id::new("toast_area"))
        .anchor(egui::Align2::RIGHT_BOTTOM, egui::vec2(-10.0, -10.0))
        .show(ctx, |ui| {
            ui.with_layout(egui::Layout::bottom_up(egui::Align::RIGHT), |ui| {
                let mut to_remove = Vec::new();
                for (i, toast) in manager.toasts.iter().enumerate() {
                    let (bg_color, text_color) = match toast.level {
                        ToastLevel::Success => (
                            egui::Color32::from_rgba_unmultiplied(76, 175, 80, 230),
                            egui::Color32::WHITE,
                        ),
                        ToastLevel::Error => (
                            egui::Color32::from_rgba_unmultiplied(244, 67, 54, 230),
                            egui::Color32::WHITE,
                        ),
                        ToastLevel::Info => (
                            egui::Color32::from_rgba_unmultiplied(80, 80, 80, 230),
                            egui::Color32::from_gray(220),
                        ),
                    };

                    let icon = match toast.level {
                        ToastLevel::Success => "✅",
                        ToastLevel::Error => "❌",
                        ToastLevel::Info => "ℹ",
                    };

                    let frame = egui::Frame::NONE
                        .fill(bg_color)
                        .corner_radius(4.0)
                        .inner_margin(egui::Margin::symmetric(12, 8));

                    frame.show(ui, |ui| {
                        ui.horizontal(|ui| {
                            ui.colored_label(text_color, icon);
                            ui.colored_label(text_color, &toast.message);
                            // 错误通知可以点击关闭
                            if toast.level == ToastLevel::Error && ui.small_button("✕").clicked()
                            {
                                to_remove.push(i);
                            }
                        });
                    });
                    ui.add_space(4.0);
                }
                // 移除被点击的 Toast（反向遍历以保持索引正确）
                for i in to_remove.into_iter().rev() {
                    manager.toasts.remove(i);
                }
            });
        });
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_toast_manager_new() {
        let manager = ToastManager::new();
        assert_eq!(manager.count(), 0);
    }

    #[test]
    fn test_toast_manager_add_success() {
        let mut manager = ToastManager::new();
        manager.success("Test success");
        assert_eq!(manager.count(), 1);
        assert_eq!(manager.toasts[0].level, ToastLevel::Success);
        assert_eq!(manager.toasts[0].message, "Test success");
    }

    #[test]
    fn test_toast_manager_add_error() {
        let mut manager = ToastManager::new();
        manager.error("Test error");
        assert_eq!(manager.count(), 1);
        assert_eq!(manager.toasts[0].level, ToastLevel::Error);
        assert_eq!(manager.toasts[0].message, "Test error");
    }

    #[test]
    fn test_toast_manager_add_info() {
        let mut manager = ToastManager::new();
        manager.info("Test info");
        assert_eq!(manager.count(), 1);
        assert_eq!(manager.toasts[0].level, ToastLevel::Info);
        assert_eq!(manager.toasts[0].message, "Test info");
    }

    #[test]
    fn test_toast_manager_multiple_toasts() {
        let mut manager = ToastManager::new();
        manager.success("First");
        manager.error("Second");
        manager.info("Third");
        assert_eq!(manager.count(), 3);
    }

    #[test]
    fn test_toast_duration_success() {
        let mut manager = ToastManager::new();
        manager.success("Test");
        assert_eq!(manager.toasts[0].duration, Duration::from_secs(3));
    }

    #[test]
    fn test_toast_duration_error() {
        let mut manager = ToastManager::new();
        manager.error("Test");
        assert_eq!(manager.toasts[0].duration, Duration::from_secs(5));
    }

    #[test]
    fn test_toast_duration_info() {
        let mut manager = ToastManager::new();
        manager.info("Test");
        assert_eq!(manager.toasts[0].duration, Duration::from_secs(3));
    }

    #[test]
    fn test_toast_manager_cleanup_empty() {
        let mut manager = ToastManager::new();
        manager.cleanup();
        assert_eq!(manager.count(), 0);
    }

    #[test]
    fn test_toast_manager_cleanup_keeps_fresh() {
        let mut manager = ToastManager::new();
        manager.success("Fresh toast");
        manager.cleanup();
        // Fresh toast should not be removed
        assert_eq!(manager.count(), 1);
    }

    #[test]
    fn test_toast_manager_default() {
        let manager = ToastManager::default();
        assert_eq!(manager.count(), 0);
    }

    #[test]
    fn test_toast_level_equality() {
        assert_eq!(ToastLevel::Success, ToastLevel::Success);
        assert_ne!(ToastLevel::Success, ToastLevel::Error);
        assert_ne!(ToastLevel::Error, ToastLevel::Info);
    }
}
