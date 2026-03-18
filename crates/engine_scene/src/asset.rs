// Copyright The SimpleGameEngine Contributors


use std::collections::HashMap;
use std::marker::PhantomData;

pub type AssetId = u64;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct Handle<T> {
    id: AssetId,
    _marker: PhantomData<T>,
}

impl<T> Handle<T> {
    pub fn id(&self) -> AssetId {
        self.id
    }
}

pub struct AssetManager {
    next_id: AssetId,
    models: HashMap<AssetId, engine_renderer::Model>,
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
    ) -> engine_renderer::Result<Handle<engine_renderer::Model>> {
        let model = engine_renderer::Model::load(path)?;
        let id = self.next_id;
        self.next_id += 1;
        self.models.insert(id, model);
        Ok(Handle {
            id,
            _marker: PhantomData,
        })
    }

    pub fn get_model(
        &self,
        handle: Handle<engine_renderer::Model>,
    ) -> Option<&engine_renderer::Model> {
        self.models.get(&handle.id)
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
            "/../engine_renderer/../../assets/models/utah-teapot-texture/teapot.obj"
        );
        if let Ok(h1) = mgr.load_model(path) {
            if let Ok(h2) = mgr.load_model(path) {
                assert_ne!(h1.id(), h2.id());
            }
        }
    }
}
