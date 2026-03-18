//! 控制台面板 — 在编辑器中显示日志消息。

use tracing::Level;

use crate::logger::{self, LogBuffer};

/// 控制台面板的 UI 状态
pub struct ConsoleState {
    pub min_level: Level,
    pub auto_scroll: bool,
}

impl Default for ConsoleState {
    fn default() -> Self {
        Self {
            min_level: Level::TRACE,
            auto_scroll: true,
        }
    }
}

pub fn show(ui: &mut egui::Ui, log_buffer: &LogBuffer, state: &mut ConsoleState) {
    ui.horizontal(|ui| {
        ui.heading("控制台");
        ui.separator();

        let levels = [
            (Level::ERROR, "错误"),
            (Level::WARN, "警告"),
            (Level::INFO, "信息"),
            (Level::DEBUG, "调试"),
            (Level::TRACE, "全部"),
        ];
        for (level, label) in &levels {
            let selected = state.min_level == *level;
            if ui.selectable_label(selected, *label).clicked() {
                state.min_level = *level;
            }
        }

        ui.separator();
        ui.checkbox(&mut state.auto_scroll, "自动滚动");

        if ui.button("清空").clicked() {
            logger::clear_log_buffer(log_buffer);
        }
    });

    ui.separator();

    let scroll = egui::ScrollArea::vertical()
        .auto_shrink([false, false])
        .stick_to_bottom(state.auto_scroll);

    scroll.show(ui, |ui| {
        if let Ok(entries) = log_buffer.lock() {
            for entry in entries.iter() {
                if entry.level > state.min_level {
                    continue;
                }

                let color = level_color(entry.level);
                let level_tag = match entry.level {
                    Level::ERROR => "[ERROR]",
                    Level::WARN => "[WARN] ",
                    Level::INFO => "[INFO] ",
                    Level::DEBUG => "[DEBUG]",
                    Level::TRACE => "[TRACE]",
                };

                let text = format!(
                    "[{:.3}] {} {}: {}",
                    entry.timestamp, level_tag, entry.target, entry.message
                );

                ui.colored_label(color, text);
            }

            if entries.is_empty() {
                ui.colored_label(egui::Color32::from_gray(100), "（暂无日志消息）");
            }
        }
    });
}

fn level_color(level: Level) -> egui::Color32 {
    match level {
        Level::ERROR => egui::Color32::from_rgb(255, 80, 80),
        Level::WARN => egui::Color32::from_rgb(255, 200, 60),
        Level::INFO => egui::Color32::from_rgb(200, 200, 200),
        Level::DEBUG => egui::Color32::from_rgb(130, 170, 255),
        Level::TRACE => egui::Color32::from_gray(120),
    }
}

#[cfg(test)]
mod tests {
    use tracing::Level;

    /// 验证 tracing::Level 的比较语义：
    /// ERROR < WARN < INFO < DEBUG < TRACE（按冗余度升序排列）
    #[test]
    fn level_ordering_matches_verbosity() {
        assert!(Level::ERROR < Level::WARN);
        assert!(Level::WARN < Level::INFO);
        assert!(Level::INFO < Level::DEBUG);
        assert!(Level::DEBUG < Level::TRACE);
    }
}
