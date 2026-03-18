use engine_core::Transform;
use glam::Vec3;
use hecs::World;

use crate::asset::AssetManager;
use crate::components::*;

pub struct Scene {
    pub world: World,
    pub assets: AssetManager,
}

impl Scene {
    pub fn new() -> Self {
        Self {
            world: World::new(),
            assets: AssetManager::new(),
        }
    }

    pub fn spawn_model(
        &mut self,
        name: &str,
        path: &str,
    ) -> engine_render_sw::Result<hecs::Entity> {
        let handle = self.assets.load_model(path)?;
        let entity = self.world.spawn((
            Name(name.to_string()),
            Transform::default(),
            MeshRenderer {
                model_handle: handle,
            },
        ));
        Ok(entity)
    }

    pub fn spawn_light(
        &mut self,
        name: &str,
        direction: Vec3,
        color: engine_render_sw::Color,
    ) -> hecs::Entity {
        self.world.spawn((
            Name(name.to_string()),
            Transform::default(),
            LightComponent {
                direction,
                color,
                intensity: 1.0,
            },
        ))
    }

    pub fn spawn_camera(&mut self, name: &str) -> hecs::Entity {
        self.world.spawn((
            Name(name.to_string()),
            Transform::default(),
            CameraComponent::default(),
        ))
    }
}

impl Default for Scene {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn scene_spawn_light_and_camera() {
        let mut scene = Scene::new();
        let light = scene.spawn_light(
            "sun",
            Vec3::new(0.0, -1.0, 0.0),
            engine_render_sw::Color::WHITE,
        );
        let cam = scene.spawn_camera("main_camera");

        assert!(scene.world.get::<&LightComponent>(light).is_ok());
        assert!(scene.world.get::<&CameraComponent>(cam).is_ok());
        assert!(scene.world.get::<&Name>(light).is_ok());
    }
}
