//! 快捷键参考窗口面板

/// 快捷键参考窗口状态
#[derive(Debug, Default)]
pub struct ShortcutsHelpState {
    pub open: bool,
}

/// 显示快捷键参考窗口
pub fn show(ctx: &egui::Context, state: &mut ShortcutsHelpState) {
    if !state.open {
        return;
    }

    egui::Window::new("快捷键参考")
        .open(&mut state.open)
        .resizable(true)
        .default_width(450.0)
        .show(ctx, |ui| {
            egui::ScrollArea::vertical().show(ui, |ui| {
                // 全局快捷键
                ui.heading("全局快捷键");
                shortcut_table(
                    ui,
                    &[
                        ("Ctrl+N", "新建项目"),
                        ("Ctrl+O", "打开项目"),
                        ("Ctrl+S", "保存项目"),
                        ("Ctrl+Shift+S", "另存为"),
                        ("Ctrl+I", "导入模型"),
                        ("Ctrl+Z", "撤销"),
                        ("Ctrl+Shift+Z", "重做"),
                        ("Delete / Backspace", "删除选中实体"),
                        ("Ctrl+D", "复制选中实体"),
                        ("F2", "重命名选中实体"),
                        ("Escape", "取消选择 / 取消操作"),
                        ("F1", "快捷键参考"),
                    ],
                );

                ui.add_space(12.0);

                // 视口快捷键
                ui.heading("视口快捷键");
                shortcut_table(
                    ui,
                    &[
                        ("Q", "选择工具"),
                        ("W", "移动工具"),
                        ("E", "旋转工具"),
                        ("R", "缩放工具"),
                        ("~ (Tilde)", "切换本地/世界坐标系"),
                        ("F", "聚焦选中实体"),
                        ("G", "切换网格显示"),
                        ("1 / 3 / 7 / 5", "视图切换"),
                        ("Home", "重置相机"),
                    ],
                );

                ui.add_space(12.0);

                // 相机快捷键
                ui.heading("相机操作");
                shortcut_table(
                    ui,
                    &[
                        ("右键 + 鼠标移动", "环视"),
                        ("右键 + WASD", "飞行"),
                        ("右键 + Q/E", "升降"),
                        ("右键 + Shift", "加速飞行"),
                        ("Alt + 左键拖拽", "环绕"),
                        ("Alt + 中键拖拽", "平移"),
                        ("Alt + 右键拖拽", "推拉"),
                        ("滚轮", "缩放"),
                        ("Shift + 滚轮", "快速缩放"),
                    ],
                );

                ui.add_space(12.0);

                // 面板快捷键
                ui.heading("面板快捷键");
                shortcut_table(
                    ui,
                    &[
                        ("Ctrl+1", "切换场景树可见性"),
                        ("Ctrl+2", "切换属性面板可见性"),
                        ("Ctrl+3", "切换控制台可见性"),
                    ],
                );
            });
        });
}

fn shortcut_table(ui: &mut egui::Ui, shortcuts: &[(&str, &str)]) {
    egui::Grid::new(ui.next_auto_id())
        .num_columns(2)
        .spacing([20.0, 4.0])
        .striped(true)
        .show(ui, |ui| {
            for (key, action) in shortcuts {
                ui.strong(*key);
                ui.label(*action);
                ui.end_row();
            }
        });
}
