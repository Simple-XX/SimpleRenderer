use crate::app::EditorState;
use crate::selection::SelectionState;

/// 显示右侧属性面板，返回是否有属性被修改。
///
/// 当有实体被选中时，显示该实体的组件属性；
/// 当没有实体被选中时，显示全局渲染/环境/相机设置。
pub fn show(
    ui: &mut egui::Ui,
    scene: &mut engine_scene::Scene,
    selection: &SelectionState,
    state: &mut EditorState,
) -> bool {
    let mut changed = false;

    ui.heading("属性");
    ui.separator();

    if let Some(entity) = selection.selected {
        // ── 选中实体：显示组件属性 ──
        changed |= show_entity_properties(ui, &mut scene.world, entity);
    } else {
        // ── 未选中实体：显示全局设置 ──
        changed |= show_global_settings(ui, state);
        ui.add_space(8.0);
        ui.separator();
        ui.label("（选中实体查看组件属性）");
    }

    changed
}

/// 显示选中实体的各组件属性
fn show_entity_properties(
    ui: &mut egui::Ui,
    world: &mut engine_core::World,
    entity: engine_core::Entity,
) -> bool {
    let mut changed = false;

    // ── 名称 ──
    if let Ok(mut name) = world.get::<&mut engine_scene::Name>(entity) {
        ui.horizontal(|ui| {
            ui.label("名称:");
            if ui.text_edit_singleline(&mut name.0).changed() {
                changed = true;
            }
        });
        ui.separator();
    }

    // ── Transform ──
    if let Ok(mut transform) = world.get::<&mut engine_core::Transform>(entity) {
        egui::CollapsingHeader::new("Transform")
            .default_open(true)
            .show(ui, |ui| {
                // 位置
                ui.horizontal(|ui| {
                    ui.label("位置 X:");
                    changed |= ui
                        .add(egui::DragValue::new(&mut transform.translation.x).speed(0.1))
                        .changed();
                    ui.label("Y:");
                    changed |= ui
                        .add(egui::DragValue::new(&mut transform.translation.y).speed(0.1))
                        .changed();
                    ui.label("Z:");
                    changed |= ui
                        .add(egui::DragValue::new(&mut transform.translation.z).speed(0.1))
                        .changed();
                });

                // 旋转（欧拉角，度数显示）
                let (yaw_rad, pitch_rad, roll_rad): (f32, f32, f32) =
                    transform.rotation.to_euler(glam::EulerRot::YXZ);
                let mut yaw = yaw_rad.to_degrees();
                let mut pitch = pitch_rad.to_degrees();
                let mut roll = roll_rad.to_degrees();
                let prev_euler = (yaw, pitch, roll);

                ui.horizontal(|ui| {
                    ui.label("旋转 X:");
                    ui.add(egui::DragValue::new(&mut pitch).speed(1.0).suffix("°"));
                    ui.label("Y:");
                    ui.add(egui::DragValue::new(&mut yaw).speed(1.0).suffix("°"));
                    ui.label("Z:");
                    ui.add(egui::DragValue::new(&mut roll).speed(1.0).suffix("°"));
                });

                if (yaw - prev_euler.0).abs() > f32::EPSILON
                    || (pitch - prev_euler.1).abs() > f32::EPSILON
                    || (roll - prev_euler.2).abs() > f32::EPSILON
                {
                    transform.rotation = glam::Quat::from_euler(
                        glam::EulerRot::YXZ,
                        yaw.to_radians(),
                        pitch.to_radians(),
                        roll.to_radians(),
                    );
                    changed = true;
                }

                // 缩放
                ui.horizontal(|ui| {
                    ui.label("缩放 X:");
                    changed |= ui
                        .add(egui::DragValue::new(&mut transform.scale.x).speed(0.1))
                        .changed();
                    ui.label("Y:");
                    changed |= ui
                        .add(egui::DragValue::new(&mut transform.scale.y).speed(0.1))
                        .changed();
                    ui.label("Z:");
                    changed |= ui
                        .add(egui::DragValue::new(&mut transform.scale.z).speed(0.1))
                        .changed();
                });
            });

        ui.add_space(4.0);
    }

    // ── 网格渲染器 ──
    if let Ok(mesh) = world.get::<&engine_scene::MeshRenderer>(entity) {
        egui::CollapsingHeader::new("网格渲染器")
            .default_open(true)
            .show(ui, |ui| {
                ui.label(format!("模型: Handle({})", mesh.model_handle.id()));
            });
        ui.add_space(4.0);
    }

    // ── 灯光 ──
    if let Ok(mut light) = world.get::<&mut engine_scene::LightComponent>(entity) {
        egui::CollapsingHeader::new("灯光")
            .default_open(true)
            .show(ui, |ui| {
                ui.horizontal(|ui| {
                    ui.label("方向 X:");
                    changed |= ui
                        .add(egui::DragValue::new(&mut light.direction.x).speed(0.1))
                        .changed();
                    ui.label("Y:");
                    changed |= ui
                        .add(egui::DragValue::new(&mut light.direction.y).speed(0.1))
                        .changed();
                    ui.label("Z:");
                    changed |= ui
                        .add(egui::DragValue::new(&mut light.direction.z).speed(0.1))
                        .changed();
                });
                ui.horizontal(|ui| {
                    ui.label("强度:");
                    changed |= ui
                        .add(
                            egui::DragValue::new(&mut light.intensity)
                                .speed(0.1)
                                .range(0.0..=10.0),
                        )
                        .changed();
                });
            });
        ui.add_space(4.0);
    }

    // ── 相机 ──
    if let Ok(mut cam) = world.get::<&mut engine_scene::CameraComponent>(entity) {
        egui::CollapsingHeader::new("相机")
            .default_open(true)
            .show(ui, |ui| {
                changed |= ui
                    .add(egui::Slider::new(&mut cam.fov, 10.0..=120.0).text("视场角"))
                    .changed();
                changed |= ui
                    .add(
                        egui::Slider::new(&mut cam.near, 0.01..=10.0)
                            .text("近裁面")
                            .logarithmic(true),
                    )
                    .changed();
                changed |= ui
                    .add(
                        egui::Slider::new(&mut cam.far, 10.0..=1000.0)
                            .text("远裁面")
                            .logarithmic(true),
                    )
                    .changed();
            });
    }

    changed
}

/// 显示全局渲染/环境/相机设置（无实体选中时）
fn show_global_settings(ui: &mut egui::Ui, state: &mut EditorState) -> bool {
    let mut changed = false;

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
