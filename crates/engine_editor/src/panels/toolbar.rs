use crate::gizmo::{CoordinateSpace, GizmoState, GizmoTool};

/// 绘制工具栏面板
pub fn show(ui: &mut egui::Ui, gizmo: &mut GizmoState) {
    ui.horizontal(|ui| {
        ui.spacing_mut().item_spacing.x = 2.0;

        let tools = [
            (GizmoTool::Select, "🔲", "选择 (Q)"),
            (GizmoTool::Translate, "↔", "移动 (W)"),
            (GizmoTool::Rotate, "🔄", "旋转 (E)"),
            (GizmoTool::Scale, "📐", "缩放 (R)"),
        ];

        for (tool, icon, tooltip) in &tools {
            let selected = gizmo.tool == *tool;
            if ui
                .selectable_label(selected, *icon)
                .on_hover_text(*tooltip)
                .clicked()
            {
                gizmo.tool = *tool;
            }
        }

        ui.add_space(8.0);
        ui.separator();
        ui.add_space(8.0);

        let space_label = match gizmo.space {
            CoordinateSpace::Local => "Local",
            CoordinateSpace::World => "World",
        };
        egui::ComboBox::from_id_salt("coord_space")
            .selected_text(space_label)
            .width(70.0)
            .show_ui(ui, |ui| {
                ui.selectable_value(&mut gizmo.space, CoordinateSpace::Local, "Local");
                ui.selectable_value(&mut gizmo.space, CoordinateSpace::World, "World");
            });

        ui.add_space(8.0);
        ui.separator();
        ui.add_space(8.0);

        ui.add_enabled(false, egui::Button::new("▶"))
            .on_hover_text("播放 (Ctrl+P) — 未来功能");
        ui.add_enabled(false, egui::Button::new("⏸"))
            .on_hover_text("暂停 — 未来功能");
        ui.add_enabled(false, egui::Button::new("⏹"))
            .on_hover_text("停止 — 未来功能");
    });
}
