use crate::selection::SelectionState;

/// 场景树面板状态
#[derive(Default)]
pub struct SceneTreeState {
    /// 搜索过滤文本
    pub search_text: String,
    /// 正在重命名的实体
    pub renaming_entity: Option<engine_core::Entity>,
    /// 重命名输入文本
    pub rename_text: String,
}

pub fn show(
    ui: &mut egui::Ui,
    scene: &mut engine_scene::Scene,
    selection: &mut SelectionState,
    state: &mut SceneTreeState,
) {
    ui.heading("场景树");
    ui.separator();

    // 搜索栏
    ui.horizontal(|ui| {
        ui.label("🔍");
        ui.add(
            egui::TextEdit::singleline(&mut state.search_text)
                .hint_text("搜索实体...")
                .desired_width(ui.available_width() - 30.0),
        );
    });
    ui.add_space(4.0);

    let search_lower = state.search_text.to_lowercase();
    let has_filter = !state.search_text.is_empty();

    // 收集实体列表（避免借用冲突）
    let entities: Vec<(engine_core::Entity, String, &'static str)> = scene
        .world
        .query::<&engine_scene::Name>()
        .iter()
        .filter(|(_entity, name)| {
            if has_filter {
                name.0.to_lowercase().contains(&search_lower)
            } else {
                true
            }
        })
        .map(|(entity, name)| {
            let icon = determine_icon_from_world(&scene.world, entity);
            (entity, name.0.clone(), icon)
        })
        .collect();

    let has_entities = !entities.is_empty();

    // 需要在滚动区域外追踪重命名结果
    let mut rename_apply: Option<(engine_core::Entity, String)> = None;
    let mut rename_cancel = false;

    egui::ScrollArea::vertical().show(ui, |ui| {
        for (entity, name, icon) in &entities {
            let is_selected = selection.is_selected(*entity);
            let is_renaming = state.renaming_entity == Some(*entity);

            ui.horizontal(|ui| {
                if is_renaming {
                    // 内联重命名模式
                    ui.label(*icon);
                    let response = ui.add(
                        egui::TextEdit::singleline(&mut state.rename_text)
                            .desired_width(ui.available_width() - 40.0),
                    );
                    // 首次显示时请求焦点
                    response.request_focus();

                    if response.lost_focus() {
                        if ui.input(|i| i.key_pressed(egui::Key::Escape)) {
                            // Escape 取消
                            rename_cancel = true;
                        } else {
                            // Enter 或失去焦点 → 应用
                            rename_apply = Some((*entity, state.rename_text.clone()));
                        }
                    }
                } else {
                    // 普通显示模式
                    let label = format!("{} {}", icon, name);
                    let response = ui.selectable_label(is_selected, label);

                    if response.clicked() {
                        selection.select(*entity);
                    }

                    // 双击进入重命名
                    if response.double_clicked() {
                        state.renaming_entity = Some(*entity);
                        state.rename_text = name.clone();
                    }

                    // 可见性占位按钮（当前无实际功能）
                    let eye = ui.small_button("👁").on_hover_text("可见性切换（尚未实现）");
                    let _ = eye;
                }
            });
        }
    });

    // 应用重命名
    if let Some((entity, new_name)) = rename_apply {
        if !new_name.is_empty() {
            if let Ok(mut name_ref) = scene.world.get::<&mut engine_scene::Name>(entity) {
                name_ref.0 = new_name;
            }
        }
        state.renaming_entity = None;
        state.rename_text.clear();
    } else if rename_cancel {
        state.renaming_entity = None;
        state.rename_text.clear();
    }

    if !has_entities {
        if has_filter {
            ui.label("（无匹配实体）");
        } else {
            ui.label("（空场景）");
        }
    }

    // 底部添加实体按钮
    ui.separator();
    if ui.button("+ 添加实体").clicked() {
        let name = format!("实体 {}", scene.world.len());
        scene
            .world
            .spawn((engine_scene::Name(name), engine_core::Transform::default()));
    }
}

/// 根据实体组件确定图标（直接操作 World 引用）
fn determine_icon_from_world(
    world: &engine_core::World,
    entity: engine_core::Entity,
) -> &'static str {
    if world.get::<&engine_scene::MeshRenderer>(entity).is_ok() {
        "🧊"
    } else if world.get::<&engine_scene::LightComponent>(entity).is_ok() {
        "💡"
    } else if world.get::<&engine_scene::CameraComponent>(entity).is_ok() {
        "📷"
    } else {
        "📦"
    }
}
