// Copyright (c) Simple-XX/SimpleRenderer
// SPDX-License-Identifier: MIT

use engine_core::{Entity, World};
use glam::Vec3;

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

    pub fn spawn_model(&mut self, name: &str, path: &str) -> engine_renderer::Result<Entity> {
        let handle = self.assets.load_model(path)?;
        let e = self.world.spawn();
        self.world.insert(e, Name(name.to_string()));
        self.world.insert(e, Transform::default());
        self.world.insert(
            e,
            MeshRenderer {
                model_handle: handle,
            },
        );
        Ok(e)
    }

    pub fn spawn_light(
        &mut self,
        name: &str,
        direction: Vec3,
        color: engine_renderer::Color,
    ) -> Entity {
        let e = self.world.spawn();
        self.world.insert(e, Name(name.to_string()));
        self.world.insert(e, Transform::default());
        self.world.insert(
            e,
            LightComponent {
                direction,
                color,
                intensity: 1.0,
            },
        );
        e
    }

    pub fn spawn_camera(&mut self, name: &str) -> Entity {
        let e = self.world.spawn();
        self.world.insert(e, Name(name.to_string()));
        self.world.insert(e, Transform::default());
        self.world.insert(e, CameraComponent::default());
        e
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
            engine_renderer::Color::WHITE,
        );
        let cam = scene.spawn_camera("main_camera");

        assert!(scene.world.get::<LightComponent>(light).is_some());
        assert!(scene.world.get::<CameraComponent>(cam).is_some());
        assert!(scene.world.get::<Name>(light).is_some());
        assert_eq!(scene.world.entities().len(), 2);
    }
}
