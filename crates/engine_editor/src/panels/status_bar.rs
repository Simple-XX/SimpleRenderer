/// 显示底部状态栏（FPS、渲染耗时等）
pub fn show(
    ui: &mut egui::Ui,
    display_fps: f32,
    render_time_ms: f32,
    rendering_mode: &str,
    model_loaded: bool,
) {
    ui.horizontal(|ui| {
        ui.label(format!("显示 FPS: {:.0}", display_fps));
        ui.separator();
        ui.label(format!("渲染耗时: {:.1} ms", render_time_ms));
        ui.separator();
        ui.label(format!("渲染模式: {}", rendering_mode));
        ui.separator();
        if model_loaded {
            ui.label("✓ 模型已加载");
        } else {
            ui.colored_label(egui::Color32::YELLOW, "⚠ 未加载模型");
        }
    });
}
