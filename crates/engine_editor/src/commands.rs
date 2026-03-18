use engine_core::{Entity, Transform};
use engine_scene::Scene;

/// 可撤销的编辑器命令
pub trait EditorCommand: Send {
    /// 执行命令
    fn execute(&mut self, scene: &mut Scene);
    /// 撤销命令
    fn undo(&mut self, scene: &mut Scene);
    /// 命令描述（用于 UI 显示）
    fn description(&self) -> &str;
}

/// 命令历史栈
pub struct CommandHistory {
    undo_stack: Vec<Box<dyn EditorCommand>>,
    redo_stack: Vec<Box<dyn EditorCommand>>,
    max_history: usize,
}

impl Default for CommandHistory {
    fn default() -> Self {
        Self {
            undo_stack: Vec::new(),
            redo_stack: Vec::new(),
            max_history: 100,
        }
    }
}

impl CommandHistory {
    /// 执行命令并加入历史
    pub fn execute(&mut self, mut cmd: Box<dyn EditorCommand>, scene: &mut Scene) {
        cmd.execute(scene);
        self.undo_stack.push(cmd);
        self.redo_stack.clear();
        if self.undo_stack.len() > self.max_history {
            self.undo_stack.remove(0);
        }
    }

    /// 撤销最近一次操作
    pub fn undo(&mut self, scene: &mut Scene) {
        if let Some(mut cmd) = self.undo_stack.pop() {
            cmd.undo(scene);
            self.redo_stack.push(cmd);
        }
    }

    /// 重做最近一次撤销
    pub fn redo(&mut self, scene: &mut Scene) {
        if let Some(mut cmd) = self.redo_stack.pop() {
            cmd.execute(scene);
            self.undo_stack.push(cmd);
        }
    }

    pub fn can_undo(&self) -> bool {
        !self.undo_stack.is_empty()
    }

    pub fn can_redo(&self) -> bool {
        !self.redo_stack.is_empty()
    }

    pub fn undo_description(&self) -> Option<&str> {
        self.undo_stack.last().map(|c| c.description())
    }

    pub fn redo_description(&self) -> Option<&str> {
        self.redo_stack.last().map(|c| c.description())
    }
}

/// 修改 Transform 命令
pub struct TransformCommand {
    pub entity: Entity,
    pub old_transform: Transform,
    pub new_transform: Transform,
    description: String,
}

impl TransformCommand {
    pub fn new(entity: Entity, old: Transform, new: Transform, name: &str) -> Self {
        Self {
            entity,
            old_transform: old,
            new_transform: new,
            description: format!("变换 {}", name),
        }
    }
}

impl EditorCommand for TransformCommand {
    fn execute(&mut self, scene: &mut Scene) {
        if let Ok(mut t) = scene.world.get::<&mut Transform>(self.entity) {
            *t = self.new_transform.clone();
        }
    }

    fn undo(&mut self, scene: &mut Scene) {
        if let Ok(mut t) = scene.world.get::<&mut Transform>(self.entity) {
            *t = self.old_transform.clone();
        }
    }

    fn description(&self) -> &str {
        &self.description
    }
}

/// 生成实体命令
pub struct SpawnCommand {
    pub entity: Option<Entity>,
    pub name: String,
    pub transform: Transform,
    description: String,
}

impl SpawnCommand {
    pub fn new(name: String, transform: Transform) -> Self {
        let description = format!("创建 {}", name);
        Self {
            entity: None,
            name,
            transform,
            description,
        }
    }
}

impl EditorCommand for SpawnCommand {
    fn execute(&mut self, scene: &mut Scene) {
        let entity = scene.world.spawn((
            engine_scene::Name(self.name.clone()),
            self.transform.clone(),
        ));
        self.entity = Some(entity);
    }

    fn undo(&mut self, scene: &mut Scene) {
        if let Some(entity) = self.entity {
            let _ = scene.world.despawn(entity);
            self.entity = None;
        }
    }

    fn description(&self) -> &str {
        &self.description
    }
}

/// 删除实体命令
pub struct DespawnCommand {
    pub entity: Entity,
    pub name: String,
    pub transform: Transform,
    description: String,
}

impl DespawnCommand {
    pub fn new(entity: Entity, name: String, transform: Transform) -> Self {
        let description = format!("删除 {}", name);
        Self {
            entity,
            name,
            transform,
            description,
        }
    }
}

impl EditorCommand for DespawnCommand {
    fn execute(&mut self, scene: &mut Scene) {
        let _ = scene.world.despawn(self.entity);
    }

    fn undo(&mut self, scene: &mut Scene) {
        self.entity = scene.world.spawn((
            engine_scene::Name(self.name.clone()),
            self.transform.clone(),
        ));
    }

    fn description(&self) -> &str {
        &self.description
    }
}

/// 重命名命令
pub struct RenameCommand {
    pub entity: Entity,
    pub old_name: String,
    pub new_name: String,
}

impl EditorCommand for RenameCommand {
    fn execute(&mut self, scene: &mut Scene) {
        if let Ok(mut name) = scene.world.get::<&mut engine_scene::Name>(self.entity) {
            name.0 = self.new_name.clone();
        }
    }

    fn undo(&mut self, scene: &mut Scene) {
        if let Ok(mut name) = scene.world.get::<&mut engine_scene::Name>(self.entity) {
            name.0 = self.old_name.clone();
        }
    }

    fn description(&self) -> &str {
        "重命名"
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use engine_core::Transform;
    use engine_scene::{Name, Scene};
    use glam::Vec3;

    #[test]
    fn command_history_undo_redo() {
        let mut scene = Scene::new();
        let entity = scene
            .world
            .spawn((Name("Test".to_string()), Transform::default()));
        let mut history = CommandHistory::default();

        let new_transform = Transform::from_translation(Vec3::new(1.0, 2.0, 3.0));
        let cmd =
            TransformCommand::new(entity, Transform::default(), new_transform.clone(), "Test");
        history.execute(Box::new(cmd), &mut scene);

        {
            let t = scene.world.get::<&Transform>(entity).unwrap();
            assert_eq!(t.translation, Vec3::new(1.0, 2.0, 3.0));
        }

        history.undo(&mut scene);
        {
            let t = scene.world.get::<&Transform>(entity).unwrap();
            assert_eq!(t.translation, Vec3::ZERO);
        }

        history.redo(&mut scene);
        {
            let t = scene.world.get::<&Transform>(entity).unwrap();
            assert_eq!(t.translation, Vec3::new(1.0, 2.0, 3.0));
        }
    }

    #[test]
    fn rename_command() {
        let mut scene = Scene::new();
        let entity = scene
            .world
            .spawn((Name("Old".to_string()), Transform::default()));
        let mut history = CommandHistory::default();

        let cmd = RenameCommand {
            entity,
            old_name: "Old".to_string(),
            new_name: "New".to_string(),
        };
        history.execute(Box::new(cmd), &mut scene);

        {
            let name = scene.world.get::<&Name>(entity).unwrap();
            assert_eq!(name.0, "New");
        }

        history.undo(&mut scene);
        {
            let name = scene.world.get::<&Name>(entity).unwrap();
            assert_eq!(name.0, "Old");
        }
    }
}
