// Copyright The SimpleGameEngine Contributors


mod app;
mod camera_control;
mod panels;
mod render_bridge;

fn main() -> eframe::Result<()> {
    env_logger::init();

    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default()
            .with_inner_size([1280.0, 720.0])
            .with_min_inner_size([800.0, 480.0]),
        ..Default::default()
    };

    eframe::run_native(
        "SimpleGameEngine 编辑器",
        options,
        Box::new(|cc| Ok(Box::new(app::EditorApp::new(cc)))),
    )
}
