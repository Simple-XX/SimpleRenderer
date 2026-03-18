// Copyright The SimpleGameEngine Contributors


//! 最小化的 Entity-Component-System 实现。
//!
//! 使用 `HashMap<TypeId, HashMap<Entity, Box<dyn Any>>>` 作为组件存储，
//! 简单正确，暂不优化性能。Entity ID 为单调递增的 u64。

use std::any::{Any, TypeId};
use std::collections::HashMap;

/// 实体标识符，单调递增的 u64。
pub type Entity = u64;

/// 组件 trait，所有组件类型必须实现此 trait。
/// 要求 `'static` 生命周期以便进行类型擦除存储。
pub trait Component: 'static {}

/// ECS 世界，管理所有实体和组件。
pub struct World {
    /// 下一个可分配的实体 ID
    next_entity: Entity,
    /// 存活的实体列表
    entities: Vec<Entity>,
    /// 组件存储：TypeId -> (Entity -> 组件数据)
    storage: HashMap<TypeId, HashMap<Entity, Box<dyn Any>>>,
}

impl Default for World {
    fn default() -> Self {
        Self::new()
    }
}

impl World {
    /// 创建一个空的 ECS 世界。
    pub fn new() -> Self {
        Self {
            next_entity: 0,
            entities: Vec::new(),
            storage: HashMap::new(),
        }
    }

    /// 生成一个新实体，返回其唯一 ID。
    pub fn spawn(&mut self) -> Entity {
        let entity = self.next_entity;
        self.next_entity += 1;
        self.entities.push(entity);
        entity
    }

    /// 销毁一个实体，移除其所有关联组件。
    pub fn despawn(&mut self, entity: Entity) {
        self.entities.retain(|&e| e != entity);
        // 从所有组件存储中移除该实体的数据
        for store in self.storage.values_mut() {
            store.remove(&entity);
        }
    }

    /// 为指定实体插入（或替换）一个组件。
    pub fn insert<C: Component>(&mut self, entity: Entity, component: C) {
        let type_id = TypeId::of::<C>();
        self.storage
            .entry(type_id)
            .or_default()
            .insert(entity, Box::new(component));
    }

    /// 获取指定实体的某类型组件的不可变引用。
    /// 如果实体没有该组件则返回 `None`。
    pub fn get<C: Component>(&self, entity: Entity) -> Option<&C> {
        let type_id = TypeId::of::<C>();
        self.storage
            .get(&type_id)?
            .get(&entity)?
            .downcast_ref::<C>()
    }

    /// 获取指定实体的某类型组件的可变引用。
    /// 如果实体没有该组件则返回 `None`。
    pub fn get_mut<C: Component>(&mut self, entity: Entity) -> Option<&mut C> {
        let type_id = TypeId::of::<C>();
        self.storage
            .get_mut(&type_id)?
            .get_mut(&entity)?
            .downcast_mut::<C>()
    }

    /// 查询拥有指定组件类型的所有实体。
    pub fn query<C: Component>(&self) -> impl Iterator<Item = Entity> + '_ {
        let type_id = TypeId::of::<C>();
        self.storage
            .get(&type_id)
            .into_iter()
            .flat_map(|store| store.keys().copied())
    }

    /// 查询同时拥有两种组件类型的所有实体。
    pub fn query_with<C1: Component, C2: Component>(&self) -> impl Iterator<Item = Entity> + '_ {
        let type_id_1 = TypeId::of::<C1>();
        let type_id_2 = TypeId::of::<C2>();
        let store_2 = self.storage.get(&type_id_2);
        self.storage
            .get(&type_id_1)
            .into_iter()
            .flat_map(move |store_1| {
                store_1
                    .keys()
                    .copied()
                    .filter(move |entity| store_2.map(|s| s.contains_key(entity)).unwrap_or(false))
            })
    }

    /// 返回所有存活实体的切片。
    pub fn entities(&self) -> &[Entity] {
        &self.entities
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // 测试用的组件类型
    struct Position {
        x: f32,
        y: f32,
    }
    impl Component for Position {}

    #[allow(dead_code)]
    struct Velocity {
        dx: f32,
        dy: f32,
    }
    impl Component for Velocity {}

    struct Health {
        hp: i32,
    }
    impl Component for Health {}

    #[test]
    fn 生成实体并插入组件然后取回() {
        let mut world = World::new();
        let e = world.spawn();
        world.insert(e, Position { x: 1.0, y: 2.0 });

        let pos = world.get::<Position>(e).unwrap();
        assert!((pos.x - 1.0).abs() < f32::EPSILON);
        assert!((pos.y - 2.0).abs() < f32::EPSILON);
    }

    #[test]
    fn 查询单一组件类型() {
        let mut world = World::new();
        let e1 = world.spawn();
        let e2 = world.spawn();
        let e3 = world.spawn();

        world.insert(e1, Position { x: 0.0, y: 0.0 });
        world.insert(e2, Position { x: 1.0, y: 1.0 });
        // e3 没有 Position 组件

        let mut entities: Vec<Entity> = world.query::<Position>().collect();
        entities.sort();
        assert_eq!(entities, vec![e1, e2]);

        // e3 不在查询结果中
        assert!(!entities.contains(&e3));
    }

    #[test]
    fn 查询两种组件类型() {
        let mut world = World::new();
        let e1 = world.spawn();
        let e2 = world.spawn();
        let e3 = world.spawn();

        world.insert(e1, Position { x: 0.0, y: 0.0 });
        world.insert(e1, Velocity { dx: 1.0, dy: 0.0 });

        world.insert(e2, Position { x: 1.0, y: 1.0 });
        // e2 只有 Position，没有 Velocity

        world.insert(e3, Velocity { dx: 2.0, dy: 2.0 });
        // e3 只有 Velocity，没有 Position

        let entities: Vec<Entity> = world.query_with::<Position, Velocity>().collect();
        assert_eq!(entities, vec![e1]);
    }

    #[test]
    fn 销毁实体后移除所有组件() {
        let mut world = World::new();
        let e = world.spawn();
        world.insert(e, Position { x: 1.0, y: 2.0 });
        world.insert(e, Velocity { dx: 3.0, dy: 4.0 });

        world.despawn(e);

        // 实体不在存活列表中
        assert!(!world.entities().contains(&e));
        // 所有组件已移除
        assert!(world.get::<Position>(e).is_none());
        assert!(world.get::<Velocity>(e).is_none());
        // 查询也找不到该实体
        assert_eq!(world.query::<Position>().count(), 0);
    }

    #[test]
    fn 获取不存在的组件返回none() {
        let mut world = World::new();
        let e = world.spawn();
        // 没有插入任何组件
        assert!(world.get::<Position>(e).is_none());
    }

    #[test]
    fn 获取错误类型的组件返回none() {
        let mut world = World::new();
        let e = world.spawn();
        world.insert(e, Position { x: 1.0, y: 2.0 });
        // 插入的是 Position，查询 Velocity 应返回 None
        assert!(world.get::<Velocity>(e).is_none());
    }

    #[test]
    fn 没有实体拥有该组件时查询返回空迭代器() {
        let world = World::new();
        assert_eq!(world.query::<Position>().count(), 0);
    }

    #[test]
    fn 可变获取并修改组件() {
        let mut world = World::new();
        let e = world.spawn();
        world.insert(e, Health { hp: 100 });

        // 通过可变引用修改组件
        if let Some(health) = world.get_mut::<Health>(e) {
            health.hp -= 25;
        }

        let health = world.get::<Health>(e).unwrap();
        assert_eq!(health.hp, 75);
    }

    #[test]
    fn 实体id单调递增() {
        let mut world = World::new();
        let e1 = world.spawn();
        let e2 = world.spawn();
        let e3 = world.spawn();
        assert!(e1 < e2);
        assert!(e2 < e3);
    }

    #[test]
    fn 插入组件会替换同类型旧值() {
        let mut world = World::new();
        let e = world.spawn();
        world.insert(e, Position { x: 1.0, y: 2.0 });
        world.insert(e, Position { x: 10.0, y: 20.0 });

        let pos = world.get::<Position>(e).unwrap();
        assert!((pos.x - 10.0).abs() < f32::EPSILON);
        assert!((pos.y - 20.0).abs() < f32::EPSILON);
    }

    #[test]
    fn entities返回所有存活实体() {
        let mut world = World::new();
        let e1 = world.spawn();
        let e2 = world.spawn();
        let e3 = world.spawn();

        assert_eq!(world.entities(), &[e1, e2, e3]);

        world.despawn(e2);
        assert_eq!(world.entities(), &[e1, e3]);
    }

    #[test]
    fn default实现与new一致() {
        let world = World::default();
        assert!(world.entities().is_empty());
        assert_eq!(world.query::<Position>().count(), 0);
    }
}
