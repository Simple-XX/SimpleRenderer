mod app;
mod camera_control;
#[allow(dead_code)]
mod commands;
#[allow(dead_code)]
mod gizmo;
mod logger;
mod panels;
mod project;
mod render_bridge;
#[allow(dead_code)]
mod selection;

fn main() -> eframe::Result<()> {
    let log_buffer = logger::init_logger(2000);

    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default()
            .with_inner_size([1280.0, 720.0])
            .with_min_inner_size([800.0, 480.0]),
        ..Default::default()
    };

    eframe::run_native(
        "SimpleGameEngine 编辑器",
        options,
        Box::new(move |cc| Ok(Box::new(app::EditorApp::new(cc, log_buffer)))),
    )
}
