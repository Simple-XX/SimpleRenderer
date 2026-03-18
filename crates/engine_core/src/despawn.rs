//! 延迟销毁队列 — 标记待销毁的实体，在安全时机统一处理。
//!
//! 避免在遍历 ECS 世界时直接销毁实体导致的迭代器失效问题。

/// 延迟销毁队列，收集待销毁实体 ID。
pub struct DespawnQueue {
    pending: Vec<hecs::Entity>,
}

impl Default for DespawnQueue {
    fn default() -> Self {
        Self::new()
    }
}

impl DespawnQueue {
    /// 创建一个空的销毁队列。
    pub fn new() -> Self {
        Self {
            pending: Vec::new(),
        }
    }

    /// 标记一个实体为待销毁。
    pub fn mark(&mut self, entity: hecs::Entity) {
        self.pending.push(entity);
    }

    /// 取出所有待销毁的实体，清空队列。
    pub fn drain(&mut self) -> Vec<hecs::Entity> {
        std::mem::take(&mut self.pending)
    }

    /// 检查队列是否为空。
    pub fn is_empty(&self) -> bool {
        self.pending.is_empty()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn 新建队列为空() {
        let queue = DespawnQueue::new();
        assert!(queue.is_empty());
    }

    #[test]
    fn mark后不再为空() {
        let mut world = hecs::World::new();
        let entity = world.spawn((1_i32,));

        let mut queue = DespawnQueue::new();
        queue.mark(entity);
        assert!(!queue.is_empty());
    }

    #[test]
    fn drain返回并清空() {
        let mut world = hecs::World::new();
        let e1 = world.spawn((1_i32,));
        let e2 = world.spawn((2_i32,));

        let mut queue = DespawnQueue::new();
        queue.mark(e1);
        queue.mark(e2);

        let entities = queue.drain();
        assert_eq!(entities.len(), 2);
        assert!(queue.is_empty());
    }

    #[test]
    fn drain后再次drain返回空() {
        let mut world = hecs::World::new();
        let entity = world.spawn((1_i32,));

        let mut queue = DespawnQueue::new();
        queue.mark(entity);

        let _ = queue.drain();
        let empty = queue.drain();
        assert!(empty.is_empty());
    }

    #[test]
    fn 配合世界销毁实体() {
        let mut world = hecs::World::new();
        let e1 = world.spawn((10_i32,));
        let e2 = world.spawn((20_i32,));

        let mut queue = DespawnQueue::new();
        queue.mark(e1);

        // 统一执行销毁
        for entity in queue.drain() {
            let _ = world.despawn(entity);
        }

        assert!(!world.contains(e1));
        assert!(world.contains(e2));
    }

    #[test]
    fn default与new一致() {
        let queue = DespawnQueue::default();
        assert!(queue.is_empty());
    }
}
