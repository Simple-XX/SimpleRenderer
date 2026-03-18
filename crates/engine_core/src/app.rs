//! 应用构建器 — 组合 ECS 世界、资源、事件、调度和插件。
//!
//! `AppBuilder` 是引擎的入口点，管理所有子系统的初始化和运行。

use crate::event::EventBus;
use crate::plugin::Plugin;
use crate::resource::ResourceMap;
use crate::schedule::{Phase, Schedule, SystemContext};

/// 应用构建器，整合引擎所有子系统。
pub struct AppBuilder {
    /// ECS 世界
    pub world: hecs::World,
    /// 全局资源
    pub resources: ResourceMap,
    /// 事件总线
    pub events: EventBus,
    /// 系统调度器
    pub schedule: Schedule,
    /// 已注册的插件
    plugins: Vec<Box<dyn Plugin>>,
    /// 插件是否已构建
    plugins_built: bool,
}

impl Default for AppBuilder {
    fn default() -> Self {
        Self::new()
    }
}

impl AppBuilder {
    /// 创建一个空的应用构建器。
    pub fn new() -> Self {
        Self {
            world: hecs::World::new(),
            resources: ResourceMap::new(),
            events: EventBus::new(),
            schedule: Schedule::new(),
            plugins: Vec::new(),
            plugins_built: false,
        }
    }

    /// 注册一个插件。
    pub fn add_plugin(&mut self, plugin: impl Plugin) -> &mut Self {
        self.plugins.push(Box::new(plugin));
        self
    }

    /// 注册一个系统到指定阶段。
    pub fn add_system(
        &mut self,
        phase: Phase,
        system: impl FnMut(&mut SystemContext) + Send + 'static,
    ) -> &mut Self {
        self.schedule.add_system(phase, system);
        self
    }

    /// 插入一个全局资源。
    pub fn insert_resource<T: Send + Sync + 'static>(&mut self, resource: T) -> &mut Self {
        self.resources.insert(resource);
        self
    }

    /// 按拓扑序构建所有已注册的插件。
    ///
    /// 通过简单的拓扑排序确保依赖的插件先构建。
    /// 如果存在循环依赖或缺失依赖，会 panic。
    pub fn build_plugins(&mut self) {
        if self.plugins_built {
            return;
        }

        // 收集插件名称到索引的映射
        let names: Vec<String> = self.plugins.iter().map(|p| p.name().to_string()).collect();

        // 拓扑排序
        let order = topological_sort(&self.plugins, &names);

        // 取出插件列表，按拓扑序构建
        let plugins = std::mem::take(&mut self.plugins);
        for idx in &order {
            plugins[*idx].build(self);
        }
        self.plugins = plugins;
        self.plugins_built = true;
    }

    /// 执行一帧：运行调度器中的所有系统，然后清空事件。
    pub fn run_once(&mut self) {
        self.schedule
            .run(&mut self.world, &mut self.resources, &mut self.events);
        self.events.clear_all();
    }
}

/// 拓扑排序：按依赖关系确定插件的构建顺序。
fn topological_sort(plugins: &[Box<dyn Plugin>], names: &[String]) -> Vec<usize> {
    use std::collections::{HashMap, VecDeque};

    let name_to_idx: HashMap<&str, usize> = names.iter().enumerate().map(|(i, n)| (n.as_str(), i)).collect();

    let n = plugins.len();
    let mut in_degree = vec![0_usize; n];
    let mut adj: Vec<Vec<usize>> = vec![Vec::new(); n];

    for (i, plugin) in plugins.iter().enumerate() {
        for dep_name in plugin.dependencies() {
            if let Some(&dep_idx) = name_to_idx.get(dep_name) {
                // dep_idx -> i（依赖的插件要先构建）
                adj[dep_idx].push(i);
                in_degree[i] += 1;
            }
            // 忽略未注册的依赖（可以扩展为 panic）
        }
    }

    let mut queue: VecDeque<usize> = VecDeque::new();
    for (i, &deg) in in_degree.iter().enumerate() {
        if deg == 0 {
            queue.push_back(i);
        }
    }

    let mut order = Vec::with_capacity(n);
    while let Some(idx) = queue.pop_front() {
        order.push(idx);
        for &next in &adj[idx] {
            in_degree[next] -= 1;
            if in_degree[next] == 0 {
                queue.push_back(next);
            }
        }
    }

    assert_eq!(
        order.len(),
        n,
        "插件存在循环依赖，无法完成拓扑排序"
    );

    order
}

#[cfg(test)]
mod tests {
    use super::*;

    struct DummyPlugin {
        name: &'static str,
    }
    impl Plugin for DummyPlugin {
        fn name(&self) -> &str {
            self.name
        }
        fn build(&self, app: &mut AppBuilder) {
            app.insert_resource::<String>(self.name.to_string());
        }
    }

    #[test]
    fn 创建空应用() {
        let app = AppBuilder::new();
        assert!(!app.plugins_built);
    }

    #[test]
    fn 插入资源并获取() {
        let mut app = AppBuilder::new();
        app.insert_resource(42_i32);
        assert_eq!(*app.resources.get::<i32>().unwrap(), 42);
    }

    #[test]
    fn 添加系统并运行() {
        let mut app = AppBuilder::new();
        app.add_system(Phase::Update, |ctx: &mut SystemContext| {
            ctx.resources.insert(100_u32);
        });
        app.run_once();
        assert_eq!(*app.resources.get::<u32>().unwrap(), 100);
    }

    #[test]
    fn build_plugins构建插件() {
        let mut app = AppBuilder::new();
        app.add_plugin(DummyPlugin { name: "alpha" });
        app.build_plugins();
        assert!(app.plugins_built);
        assert_eq!(app.resources.get::<String>().unwrap(), "alpha");
    }

    #[test]
    fn build_plugins重复调用安全() {
        let mut app = AppBuilder::new();
        app.add_plugin(DummyPlugin { name: "alpha" });
        app.build_plugins();
        app.build_plugins(); // 不应 panic
        assert!(app.plugins_built);
    }

    struct DepPlugin;
    impl Plugin for DepPlugin {
        fn name(&self) -> &str {
            "dep_plugin"
        }
        fn dependencies(&self) -> Vec<&str> {
            vec!["base_plugin"]
        }
        fn build(&self, app: &mut AppBuilder) {
            // 读取基础插件设置的资源
            let base_val = *app.resources.get::<u64>().unwrap_or(&0);
            app.insert_resource(base_val + 100);
        }
    }

    struct BasePlugin;
    impl Plugin for BasePlugin {
        fn name(&self) -> &str {
            "base_plugin"
        }
        fn build(&self, app: &mut AppBuilder) {
            app.insert_resource(1_u64);
        }
    }

    #[test]
    fn 插件依赖拓扑排序() {
        let mut app = AppBuilder::new();
        // 先注册有依赖的插件，后注册被依赖的插件
        app.add_plugin(DepPlugin);
        app.add_plugin(BasePlugin);
        app.build_plugins();

        // base_plugin 先构建，设置 u64 = 1
        // dep_plugin 后构建，读取 u64 = 1，设置为 101
        assert_eq!(*app.resources.get::<u64>().unwrap(), 101);
    }

    #[test]
    fn run_once执行后清空事件() {
        use crate::event::Event;

        #[derive(Debug)]
        struct TestEvent;
        impl Event for TestEvent {}

        let mut app = AppBuilder::new();
        app.add_system(Phase::Update, |ctx: &mut SystemContext| {
            ctx.events.send(TestEvent);
        });
        app.run_once();

        // 事件应在 run_once 末尾被清空
        assert_eq!(app.events.read::<TestEvent>().count(), 0);
    }

    #[test]
    fn default与new一致() {
        let app = AppBuilder::default();
        assert!(!app.plugins_built);
    }
}
