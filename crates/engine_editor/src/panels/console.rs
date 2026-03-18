//! 控制台面板 — 在编辑器中显示日志消息。

use tracing::Level;

use crate::logger::{self, LogBuffer};

/// 控制台面板的 UI 状态
pub struct ConsoleState {
    pub min_level: Level,
    pub auto_scroll: bool,
    pub search_text: String, // 搜索过滤文本
}

impl Default for ConsoleState {
    fn default() -> Self {
        Self {
            min_level: Level::TRACE,
            auto_scroll: true,
            search_text: String::new(),
        }
    }
}

pub fn show(ui: &mut egui::Ui, log_buffer: &LogBuffer, state: &mut ConsoleState) {
    // 头部行：标题、级别过滤、自动滚动、清空
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

    // 搜索过滤栏
    ui.horizontal(|ui| {
        ui.label("🔍");
        ui.add(
            egui::TextEdit::singleline(&mut state.search_text)
                .hint_text("搜索日志...")
                .desired_width(200.0),
        );
        if !state.search_text.is_empty() && ui.small_button("✕").clicked() {
            state.search_text.clear();
        }
    });

    ui.separator();

    let scroll = egui::ScrollArea::vertical()
        .auto_shrink([false, false])
        .stick_to_bottom(state.auto_scroll);

    scroll.show(ui, |ui| {
        if let Ok(entries) = log_buffer.lock() {
            for entry in entries.iter() {
                // 级别过滤
                if entry.level > state.min_level {
                    continue;
                }

                // 搜索过滤：检查消息和目标是否包含搜索文本（不区分大小写）
                if !state.search_text.is_empty() {
                    let search_lower = state.search_text.to_lowercase();
                    if !entry.message.to_lowercase().contains(&search_lower)
                        && !entry.target.to_lowercase().contains(&search_lower)
                    {
                        continue;
                    }
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

                // 右键菜单：复制此条日志
                let response = ui.colored_label(color, &text);
                response.context_menu(|ui| {
                    if ui.button("📋 复制此条").clicked() {
                        ui.ctx().copy_text(text.clone());
                        ui.close_menu();
                    }
                });
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
