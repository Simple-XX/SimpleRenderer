//! 事件总线 — 基于类型的事件发送与读取。
//!
//! 每种事件类型对应一个 `Vec<E>` 通道，每帧结束时调用 `clear_all` 清空。

use std::any::{Any, TypeId};
use std::collections::HashMap;

/// 事件 trait，所有事件类型必须实现此 trait。
pub trait Event: Send + Sync + 'static {}

/// 事件总线，按类型存储事件通道。
pub struct EventBus {
    channels: HashMap<TypeId, Box<dyn Any + Send + Sync>>,
}

impl Default for EventBus {
    fn default() -> Self {
        Self::new()
    }
}

impl EventBus {
    /// 创建一个空的事件总线。
    pub fn new() -> Self {
        Self {
            channels: HashMap::new(),
        }
    }

    /// 发送一个事件到对应类型的通道。
    pub fn send<E: Event>(&mut self, event: E) {
        let type_id = TypeId::of::<E>();
        let channel = self
            .channels
            .entry(type_id)
            .or_insert_with(|| Box::new(Vec::<E>::new()));
        channel.downcast_mut::<Vec<E>>().unwrap().push(event);
    }

    /// 读取指定类型的所有事件。
    pub fn read<E: Event>(&self) -> impl Iterator<Item = &E> {
        self.channels
            .get(&TypeId::of::<E>())
            .and_then(|channel| channel.downcast_ref::<Vec<E>>())
            .map(|vec| vec.iter())
            .unwrap_or_else(|| [].iter())
    }

    /// 清空所有事件通道。
    pub fn clear_all(&mut self) {
        self.channels.clear();
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[derive(Debug, PartialEq)]
    struct DamageEvent {
        amount: i32,
    }
    impl Event for DamageEvent {}

    #[derive(Debug, PartialEq)]
    struct HealEvent {
        amount: i32,
    }
    impl Event for HealEvent {}

    #[test]
    fn 发送并读取事件() {
        let mut bus = EventBus::new();
        bus.send(DamageEvent { amount: 10 });
        bus.send(DamageEvent { amount: 20 });

        let events: Vec<&DamageEvent> = bus.read::<DamageEvent>().collect();
        assert_eq!(events.len(), 2);
        assert_eq!(events[0].amount, 10);
        assert_eq!(events[1].amount, 20);
    }

    #[test]
    fn 读取空通道返回空迭代器() {
        let bus = EventBus::new();
        let count = bus.read::<DamageEvent>().count();
        assert_eq!(count, 0);
    }

    #[test]
    fn 不同类型互不干扰() {
        let mut bus = EventBus::new();
        bus.send(DamageEvent { amount: 5 });
        bus.send(HealEvent { amount: 15 });

        assert_eq!(bus.read::<DamageEvent>().count(), 1);
        assert_eq!(bus.read::<HealEvent>().count(), 1);
    }

    #[test]
    fn clear_all清空所有通道() {
        let mut bus = EventBus::new();
        bus.send(DamageEvent { amount: 10 });
        bus.send(HealEvent { amount: 5 });

        bus.clear_all();

        assert_eq!(bus.read::<DamageEvent>().count(), 0);
        assert_eq!(bus.read::<HealEvent>().count(), 0);
    }

    #[test]
    fn default与new一致() {
        let bus = EventBus::default();
        assert_eq!(bus.read::<DamageEvent>().count(), 0);
    }
}
