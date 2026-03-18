//! 资产系统 — AssetId、Handle<T> 句柄和 AssetLoader trait。
//!
//! 提供类型安全的资产句柄和可扩展的加载器接口。

use std::marker::PhantomData;
use std::path::Path;

/// 资产唯一标识符。
pub type AssetId = u64;

/// 类型安全的资产句柄。
///
/// 通过泛型参数 `T` 区分不同类型的资产，编译期保证类型安全。
pub struct Handle<T> {
    id: AssetId,
    _marker: PhantomData<T>,
}

// 手动实现 trait，避免 derive 将 bounds 泄漏到类型参数 T
impl<T> std::fmt::Debug for Handle<T> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("Handle").field("id", &self.id).finish()
    }
}

impl<T> Clone for Handle<T> {
    fn clone(&self) -> Self {
        *self
    }
}

impl<T> Copy for Handle<T> {}

impl<T> PartialEq for Handle<T> {
    fn eq(&self, other: &Self) -> bool {
        self.id == other.id
    }
}

impl<T> Eq for Handle<T> {}

impl<T> std::hash::Hash for Handle<T> {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        self.id.hash(state);
    }
}

impl<T> Handle<T> {
    /// 从资产 ID 创建句柄。
    pub fn new(id: AssetId) -> Self {
        Self {
            id,
            _marker: PhantomData,
        }
    }

    /// 获取句柄对应的资产 ID。
    pub fn id(&self) -> AssetId {
        self.id
    }
}

/// 资产加载器 trait，定义如何从文件路径加载特定类型的资产。
pub trait AssetLoader: Send + Sync + 'static {
    /// 加载器产出的资产类型。
    type Asset: Send + Sync + 'static;

    /// 该加载器支持的文件扩展名列表。
    fn extensions(&self) -> &[&str];

    /// 从文件路径加载资产。
    fn load(&self, path: &Path) -> Result<Self::Asset, Box<dyn std::error::Error + Send + Sync>>;
}

#[cfg(test)]
mod tests {
    use super::*;

    struct Mesh;
    struct Texture;

    #[test]
    fn handle创建和获取id() {
        let handle = Handle::<Mesh>::new(42);
        assert_eq!(handle.id(), 42);
    }

    #[test]
    fn 不同id的handle不相等() {
        let h1 = Handle::<Mesh>::new(1);
        let h2 = Handle::<Mesh>::new(2);
        assert_ne!(h1, h2);
    }

    #[test]
    fn 相同id的handle相等() {
        let h1 = Handle::<Mesh>::new(5);
        let h2 = Handle::<Mesh>::new(5);
        assert_eq!(h1, h2);
    }

    #[test]
    fn handle可以clone() {
        let h1 = Handle::<Texture>::new(10);
        let h2 = h1;
        assert_eq!(h1.id(), h2.id());
    }

    #[test]
    fn handle可以作为hashmap的key() {
        use std::collections::HashMap;
        let mut map = HashMap::new();
        let handle = Handle::<Mesh>::new(1);
        map.insert(handle, "mesh_data");
        assert_eq!(map.get(&handle), Some(&"mesh_data"));
    }

    /// 验证 AssetLoader trait 可以被实现
    struct TestLoader;
    impl AssetLoader for TestLoader {
        type Asset = String;

        fn extensions(&self) -> &[&str] {
            &["txt", "text"]
        }

        fn load(
            &self,
            path: &Path,
        ) -> Result<Self::Asset, Box<dyn std::error::Error + Send + Sync>> {
            Ok(format!("loaded: {}", path.display()))
        }
    }

    #[test]
    fn asset_loader实现有效() {
        let loader = TestLoader;
        assert_eq!(loader.extensions(), &["txt", "text"]);
        let result = loader.load(Path::new("test.txt")).unwrap();
        assert_eq!(result, "loaded: test.txt");
    }

    #[test]
    fn asset_loader错误处理() {
        struct FailLoader;
        impl AssetLoader for FailLoader {
            type Asset = ();
            fn extensions(&self) -> &[&str] {
                &["fail"]
            }
            fn load(
                &self,
                _path: &Path,
            ) -> Result<Self::Asset, Box<dyn std::error::Error + Send + Sync>> {
                Err("load failed".into())
            }
        }

        let loader = FailLoader;
        assert!(loader.load(Path::new("test.fail")).is_err());
    }
}
