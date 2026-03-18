//! 调度系统 — Phase 阶段枚举 + Schedule 系统调度器。
//!
//! 系统按 Phase 排序后依次执行，每个系统接收 `SystemContext` 访问 ECS 世界和资源。

use crate::event::EventBus;
use crate::resource::ResourceMap;

/// 系统执行阶段，按枚举定义顺序排序。
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum Phase {
    /// 更新前的准备阶段
    PreUpdate,
    /// 主要逻辑更新阶段
    Update,
    /// 更新后的收尾阶段
    PostUpdate,
    /// 渲染前的准备阶段
    PreRender,
    /// 主要渲染阶段
    Render,
    /// 渲染后的收尾阶段
    PostRender,
}

/// 系统上下文，提供对 ECS 世界、资源和事件总线的可变访问。
pub struct SystemContext<'a> {
    pub world: &'a mut hecs::World,
    pub resources: &'a mut ResourceMap,
    pub events: &'a mut EventBus,
}

type SystemFn = Box<dyn FnMut(&mut SystemContext) + Send>;

/// 系统调度器，按阶段排序后依次执行注册的系统。
pub struct Schedule {
    systems: Vec<(Phase, SystemFn)>,
}

impl Default for Schedule {
    fn default() -> Self {
        Self::new()
    }
}

impl Schedule {
    /// 创建一个空的调度器。
    pub fn new() -> Self {
        Self {
            systems: Vec::new(),
        }
    }

    /// 注册一个系统到指定阶段。
    pub fn add_system(
        &mut self,
        phase: Phase,
        system: impl FnMut(&mut SystemContext) + Send + 'static,
    ) {
        self.systems.push((phase, Box::new(system)));
    }

    /// 按阶段排序后依次执行所有系统。
    ///
    /// 同一阶段内的系统保持注册顺序（稳定排序）。
    pub fn run(
        &mut self,
        world: &mut hecs::World,
        resources: &mut ResourceMap,
        events: &mut EventBus,
    ) {
        // 按 Phase 稳定排序，保持同阶段注册顺序
        self.systems.sort_by_key(|(phase, _)| *phase);

        for (_, system) in self.systems.iter_mut() {
            let mut ctx = SystemContext {
                world,
                resources,
                events,
            };
            system(&mut ctx);
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::{Arc, Mutex};

    #[test]
    fn 按阶段顺序执行系统() {
        let log = Arc::new(Mutex::new(Vec::<&str>::new()));

        let mut schedule = Schedule::new();

        let log_clone = Arc::clone(&log);
        schedule.add_system(Phase::Render, move |_ctx| {
            log_clone.lock().unwrap().push("render");
        });

        let log_clone = Arc::clone(&log);
        schedule.add_system(Phase::PreUpdate, move |_ctx| {
            log_clone.lock().unwrap().push("pre_update");
        });

        let log_clone = Arc::clone(&log);
        schedule.add_system(Phase::Update, move |_ctx| {
            log_clone.lock().unwrap().push("update");
        });

        let mut world = hecs::World::new();
        let mut resources = ResourceMap::new();
        let mut events = EventBus::new();

        schedule.run(&mut world, &mut resources, &mut events);

        let result = log.lock().unwrap();
        assert_eq!(*result, vec!["pre_update", "update", "render"]);
    }

    #[test]
    fn 同阶段保持注册顺序() {
        let log = Arc::new(Mutex::new(Vec::<&str>::new()));

        let mut schedule = Schedule::new();

        let log_clone = Arc::clone(&log);
        schedule.add_system(Phase::Update, move |_ctx| {
            log_clone.lock().unwrap().push("first");
        });

        let log_clone = Arc::clone(&log);
        schedule.add_system(Phase::Update, move |_ctx| {
            log_clone.lock().unwrap().push("second");
        });

        let mut world = hecs::World::new();
        let mut resources = ResourceMap::new();
        let mut events = EventBus::new();

        schedule.run(&mut world, &mut resources, &mut events);

        let result = log.lock().unwrap();
        assert_eq!(*result, vec!["first", "second"]);
    }

    #[test]
    fn 系统可以访问world() {
        let mut schedule = Schedule::new();

        schedule.add_system(Phase::Update, |ctx: &mut SystemContext| {
            ctx.world.spawn((42_i32,));
        });

        let mut world = hecs::World::new();
        let mut resources = ResourceMap::new();
        let mut events = EventBus::new();

        schedule.run(&mut world, &mut resources, &mut events);

        let count = world.query::<&i32>().iter().count();
        assert_eq!(count, 1);
    }

    #[test]
    fn 系统可以访问resources() {
        let mut schedule = Schedule::new();

        schedule.add_system(Phase::Update, |ctx: &mut SystemContext| {
            ctx.resources.insert(99_u32);
        });

        let mut world = hecs::World::new();
        let mut resources = ResourceMap::new();
        let mut events = EventBus::new();

        schedule.run(&mut world, &mut resources, &mut events);

        assert_eq!(*resources.get::<u32>().unwrap(), 99);
    }

    #[test]
    fn 空调度器运行无副作用() {
        let mut schedule = Schedule::new();
        let mut world = hecs::World::new();
        let mut resources = ResourceMap::new();
        let mut events = EventBus::new();

        schedule.run(&mut world, &mut resources, &mut events);
        // 不应 panic
    }

    #[test]
    fn phase枚举排序正确() {
        assert!(Phase::PreUpdate < Phase::Update);
        assert!(Phase::Update < Phase::PostUpdate);
        assert!(Phase::PostUpdate < Phase::PreRender);
        assert!(Phase::PreRender < Phase::Render);
        assert!(Phase::Render < Phase::PostRender);
    }

    #[test]
    fn default与new一致() {
        let schedule = Schedule::default();
        assert_eq!(schedule.systems.len(), 0);
    }
}
