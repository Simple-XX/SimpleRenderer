//! 全局资源存储 — 基于 TypeMap 模式的 ResourceMap。
//!
//! 按类型存储唯一资源，供系统在每帧中读写。

use std::any::{Any, TypeId};
use std::collections::HashMap;

/// 全局资源映射表，按类型存储唯一资源实例。
pub struct ResourceMap {
    map: HashMap<TypeId, Box<dyn Any + Send + Sync>>,
}

impl Default for ResourceMap {
    fn default() -> Self {
        Self::new()
    }
}

impl ResourceMap {
    /// 创建一个空的资源映射表。
    pub fn new() -> Self {
        Self {
            map: HashMap::new(),
        }
    }

    /// 插入一个资源，如果同类型已存在则替换。
    pub fn insert<T: Send + Sync + 'static>(&mut self, resource: T) {
        self.map.insert(TypeId::of::<T>(), Box::new(resource));
    }

    /// 获取指定类型资源的不可变引用。
    pub fn get<T: Send + Sync + 'static>(&self) -> Option<&T> {
        self.map
            .get(&TypeId::of::<T>())
            .and_then(|boxed| boxed.downcast_ref::<T>())
    }

    /// 获取指定类型资源的可变引用。
    pub fn get_mut<T: Send + Sync + 'static>(&mut self) -> Option<&mut T> {
        self.map
            .get_mut(&TypeId::of::<T>())
            .and_then(|boxed| boxed.downcast_mut::<T>())
    }

    /// 检查是否包含指定类型的资源。
    pub fn contains<T: Send + Sync + 'static>(&self) -> bool {
        self.map.contains_key(&TypeId::of::<T>())
    }

    /// 移除并返回指定类型的资源。
    pub fn remove<T: Send + Sync + 'static>(&mut self) -> Option<T> {
        self.map
            .remove(&TypeId::of::<T>())
            .and_then(|boxed| boxed.downcast::<T>().ok())
            .map(|boxed| *boxed)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn 插入并获取资源() {
        let mut map = ResourceMap::new();
        map.insert(42_i32);
        assert_eq!(*map.get::<i32>().unwrap(), 42);
    }

    #[test]
    fn 获取不存在的资源返回none() {
        let map = ResourceMap::new();
        assert!(map.get::<i32>().is_none());
    }

    #[test]
    fn 可变获取并修改() {
        let mut map = ResourceMap::new();
        map.insert(String::from("hello"));
        map.get_mut::<String>().unwrap().push_str(" world");
        assert_eq!(map.get::<String>().unwrap(), "hello world");
    }

    #[test]
    fn 插入同类型会替换() {
        let mut map = ResourceMap::new();
        map.insert(1_u64);
        map.insert(2_u64);
        assert_eq!(*map.get::<u64>().unwrap(), 2);
    }

    #[test]
    fn contains检查() {
        let mut map = ResourceMap::new();
        assert!(!map.contains::<f32>());
        map.insert(1.0_f32);
        assert!(map.contains::<f32>());
    }

    #[test]
    fn remove移除并返回() {
        let mut map = ResourceMap::new();
        map.insert(100_u32);
        let val = map.remove::<u32>().unwrap();
        assert_eq!(val, 100);
        assert!(!map.contains::<u32>());
    }

    #[test]
    fn remove不存在的类型返回none() {
        let mut map = ResourceMap::new();
        assert!(map.remove::<i64>().is_none());
    }

    #[test]
    fn 多种类型互不干扰() {
        let mut map = ResourceMap::new();
        map.insert(42_i32);
        map.insert(std::f64::consts::PI);
        map.insert(String::from("test"));

        assert_eq!(*map.get::<i32>().unwrap(), 42);
        assert!((*map.get::<f64>().unwrap() - std::f64::consts::PI).abs() < f64::EPSILON);
        assert_eq!(map.get::<String>().unwrap(), "test");
    }

    #[test]
    fn default与new一致() {
        let map = ResourceMap::default();
        assert!(!map.contains::<i32>());
    }
}
