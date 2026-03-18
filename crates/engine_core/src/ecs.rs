//! ECS 模块 — 基于 hecs 的实体组件系统。
//!
//! 直接重导出 hecs 的核心类型，无需自定义封装。

pub use hecs::Entity;
pub use hecs::World;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn 创建世界并生成实体() {
        let mut world = World::new();
        let entity = world.spawn((42_i32, std::f64::consts::PI));
        assert!(world.contains(entity));
    }

    #[test]
    fn 获取组件() {
        let mut world = World::new();
        let entity = world.spawn((42_i32, "hello"));

        let val = world.get::<&i32>(entity).unwrap();
        assert_eq!(*val, 42);
    }

    #[test]
    fn 销毁实体后不再存在() {
        let mut world = World::new();
        let entity = world.spawn((1_u32,));
        assert!(world.contains(entity));

        world.despawn(entity).unwrap();
        assert!(!world.contains(entity));
    }

    #[test]
    fn 查询组件() {
        let mut world = World::new();
        world.spawn((10_i32, true));
        world.spawn((20_i32, false));
        world.spawn(("no_int",));

        let sum: i32 = world.query::<&i32>().iter().map(|(_, v)| *v).sum();
        assert_eq!(sum, 30);
    }

    #[test]
    fn 空世界查询返回空() {
        let world = World::new();
        let count = world.query::<&i32>().iter().count();
        assert_eq!(count, 0);
    }
}
