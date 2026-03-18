// Copyright The SimpleGameEngine Contributors


use std::sync::Arc;

use crate::material::Material;

/// 由三个顶点索引和关联材质定义的三角面。
#[derive(Debug, Clone)]
pub struct Face {
    pub indices: [usize; 3],
    pub material: Arc<Material>,
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn face_creation() {
        let f = Face {
            indices: [0, 1, 2],
            material: Arc::new(Material::default()),
        };
        assert_eq!(f.indices, [0, 1, 2]);
    }

    #[test]
    fn face_clone() {
        let f = Face {
            indices: [3, 4, 5],
            material: Arc::new(Material::default()),
        };
        let f2 = f.clone();
        assert_eq!(f2.indices, [3, 4, 5]);
    }
}
