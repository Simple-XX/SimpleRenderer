pub fn show(
    ui: &mut egui::Ui,
    display_fps: f32,
    render_time_ms: f32,
    rendering_mode: &str,
    model_loaded: bool,
    project_name: &str,
    project_dirty: bool,
) {
    ui.horizontal(|ui| {
        if !project_name.is_empty() {
            let label = if project_dirty {
                format!("📁 {}*", project_name)
            } else {
                format!("📁 {}", project_name)
            };
            ui.label(label);
            ui.separator();
        }

        ui.label(format!("FPS: {:.0}", display_fps));
        ui.separator();
        ui.label(format!("渲染: {:.1} ms", render_time_ms));
        ui.separator();
        ui.label(rendering_mode);
        ui.separator();
        if model_loaded {
            ui.label("✓ 模型已加载");
        } else {
            ui.colored_label(egui::Color32::YELLOW, "⚠ 未加载模型");
        }
    });
}
