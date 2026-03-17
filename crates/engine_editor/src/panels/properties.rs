// Copyright (c) Simple-XX/SimpleRenderer
// SPDX-License-Identifier: MIT

use crate::app::EditorState;

/// 显示右侧属性面板，返回是否有属性被修改
pub fn show(ui: &mut egui::Ui, state: &mut EditorState) -> bool {
    let mut changed = false;

    ui.heading("属性");
    ui.separator();

    // ── 渲染模式 ──
    egui::CollapsingHeader::new("渲染设置")
        .default_open(true)
        .show(ui, |ui| {
            let mode_names = ["PerTriangle", "TileBased", "Deferred", "TileBasedDeferred"];
            let prev_mode = state.rendering_mode_index;
            egui::ComboBox::from_label("渲染模式")
                .selected_text(mode_names[state.rendering_mode_index])
                .show_ui(ui, |ui| {
                    for (i, name) in mode_names.iter().enumerate() {
                        ui.selectable_value(&mut state.rendering_mode_index, i, *name);
                    }
                });
            if state.rendering_mode_index != prev_mode {
                changed = true;
                state.rendering_mode_changed = true;
            }

            let prev_tile = state.tile_size;
            ui.add(egui::Slider::new(&mut state.tile_size, 16..=256).text("瓦片大小"));
            if state.tile_size != prev_tile {
                changed = true;
                state.tile_size_changed = true;
            }

            let prev_ez = state.early_z;
            ui.checkbox(&mut state.early_z, "Early-Z 优化");
            if state.early_z != prev_ez {
                changed = true;
                state.early_z_changed = true;
            }

            let prev_vsync = state.vsync;
            ui.checkbox(&mut state.vsync, "垂直同步 (VSync)");
            if state.vsync != prev_vsync {
                changed = true;
            }
        });

    ui.add_space(8.0);

    // ── 光源设置 ──
    egui::CollapsingHeader::new("光源")
        .default_open(true)
        .show(ui, |ui| {
            let prev = state.light_direction;
            ui.horizontal(|ui| {
                ui.label("方向 X:");
                ui.add(egui::DragValue::new(&mut state.light_direction[0]).speed(0.1));
            });
            ui.horizontal(|ui| {
                ui.label("方向 Y:");
                ui.add(egui::DragValue::new(&mut state.light_direction[1]).speed(0.1));
            });
            ui.horizontal(|ui| {
                ui.label("方向 Z:");
                ui.add(egui::DragValue::new(&mut state.light_direction[2]).speed(0.1));
            });
            if state.light_direction != prev {
                changed = true;
                state.light_changed = true;
            }

            let prev_color = state.light_color;
            ui.horizontal(|ui| {
                ui.label("颜色:");
                ui.color_edit_button_srgba(&mut state.light_color);
            });
            if state.light_color != prev_color {
                changed = true;
                state.light_changed = true;
            }
        });

    ui.add_space(8.0);

    // ── 材质设置 ──
    egui::CollapsingHeader::new("材质")
        .default_open(true)
        .show(ui, |ui| {
            let prev = state.shininess;
            ui.add(
                egui::Slider::new(&mut state.shininess, 1.0..=256.0)
                    .text("高光度")
                    .logarithmic(true),
            );
            if (state.shininess - prev).abs() > f32::EPSILON {
                changed = true;
                state.shininess_changed = true;
            }
        });

    ui.add_space(8.0);

    // ── 相机设置 ──
    egui::CollapsingHeader::new("相机")
        .default_open(false)
        .show(ui, |ui| {
            ui.add(egui::Slider::new(&mut state.camera_speed, 0.5..=20.0).text("移动速度"));
            ui.add(egui::Slider::new(&mut state.camera_fov, 30.0..=120.0).text("视场角"));
        });

    changed
}
