// Copyright (c) Simple-XX/SimpleRenderer
// SPDX-License-Identifier: MIT

/// 显示场景树面板（左侧实体列表）
pub fn show(ui: &mut egui::Ui, entities: &[String], selected: &mut Option<usize>) {
    ui.heading("场景树");
    ui.separator();

    if entities.is_empty() {
        ui.label("（空场景）");
        return;
    }

    egui::ScrollArea::vertical().show(ui, |ui| {
        for (i, name) in entities.iter().enumerate() {
            let is_selected = *selected == Some(i);
            if ui.selectable_label(is_selected, name).clicked() {
                *selected = Some(i);
            }
        }
    });
}
