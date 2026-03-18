use std::collections::HashMap;

use engine_core::{AssetId, Handle};

pub struct AssetManager {
    next_id: AssetId,
    models: HashMap<AssetId, engine_render_sw::Model>,
}

impl AssetManager {
    pub fn new() -> Self {
        Self {
            next_id: 1,
            models: HashMap::new(),
        }
    }

    pub fn load_model(
        &mut self,
        path: &str,
    ) -> engine_render_sw::Result<Handle<engine_render_sw::Model>> {
        let model = engine_render_sw::Model::load(path)?;
        let id = self.next_id;
        self.next_id += 1;
        self.models.insert(id, model);
        Ok(Handle::new(id))
    }

    pub fn get_model(
        &self,
        handle: Handle<engine_render_sw::Model>,
    ) -> Option<&engine_render_sw::Model> {
        self.models.get(&handle.id())
    }
}

impl Default for AssetManager {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn asset_manager_new_is_empty() {
        let mgr = AssetManager::new();
        assert!(mgr.models.is_empty());
    }

    #[test]
    fn handle_ids_are_unique() {
        let mut mgr = AssetManager::new();
        let path = concat!(
            env!("CARGO_MANIFEST_DIR"),
            "/../engine_render_sw/../../assets/models/utah-teapot-texture/teapot.obj"
        );
        if let Ok(h1) = mgr.load_model(path) {
            if let Ok(h2) = mgr.load_model(path) {
                assert_ne!(h1.id(), h2.id());
            }
        }
    }
}
