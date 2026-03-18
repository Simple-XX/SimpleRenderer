#[allow(clippy::too_many_arguments)]
pub fn show(
    ui: &mut egui::Ui,
    display_fps: f32,
    render_time_ms: f32,
    rendering_mode: &str,
    model_loaded: bool,
    project_name: &str,
    project_dirty: bool,
    selected_entity_name: Option<&str>,
    selected_entity_pos: Option<[f32; 3]>,
    undo_description: Option<&str>,
    redo_description: Option<&str>,
) {
    ui.horizontal(|ui| {
        // 项目名称和脏标记
        if !project_name.is_empty() {
            let label = if project_dirty {
                format!("📁 {}*", project_name)
            } else {
                format!("📁 {}", project_name)
            };
            ui.label(label);
            ui.separator();
        }

        // FPS 和渲染时间
        ui.label(format!("FPS: {:.0}", display_fps));
        ui.separator();
        ui.label(format!("渲染: {:.1} ms", render_time_ms));
        ui.separator();

        // 渲染模式
        ui.label(rendering_mode);
        ui.separator();

        // 模型加载状态
        if model_loaded {
            ui.label("✓ 模型已加载");
        } else {
            ui.colored_label(egui::Color32::YELLOW, "⚠ 未加载模型");
        }

        // 选中实体信息
        if let Some(name) = selected_entity_name {
            ui.separator();
            ui.label(format!("选中: {}", name));
        }

        // 选中实体位置
        if let Some(pos) = selected_entity_pos {
            ui.separator();
            ui.label(format!("({:.1}, {:.1}, {:.1})", pos[0], pos[1], pos[2]));
        }

        // 撤销预览
        if let Some(desc) = undo_description {
            ui.separator();
            ui.label(format!("↩️撤销: {}", desc));
        }

        // 重做预览
        if let Some(desc) = redo_description {
            ui.separator();
            ui.label(format!("↪️重做: {}", desc));
        }
    });
}
