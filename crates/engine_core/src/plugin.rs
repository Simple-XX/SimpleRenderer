//! 插件系统 — Plugin trait，支持依赖声明和生命周期管理。
//!
//! 插件通过 `build` 方法向 AppBuilder 注册资源、系统和事件处理。

use crate::app::AppBuilder;

/// 插件 trait，定义插件的名称、依赖和构建/清理行为。
pub trait Plugin: Send + Sync + 'static {
    /// 插件名称，用于依赖解析和日志输出。
    fn name(&self) -> &str;

    /// 声明依赖的其他插件名称列表。
    ///
    /// 默认无依赖。AppBuilder 会在 `build_plugins` 时按拓扑序构建。
    fn dependencies(&self) -> Vec<&str> {
        vec![]
    }

    /// 构建插件：向 AppBuilder 注册资源、系统等。
    fn build(&self, app: &mut AppBuilder);

    /// 清理插件（可选），在应用退出时调用。
    fn cleanup(&self, _app: &mut AppBuilder) {}
}

#[cfg(test)]
mod tests {
    use super::*;

    struct TestPlugin;
    impl Plugin for TestPlugin {
        fn name(&self) -> &str {
            "test_plugin"
        }

        fn build(&self, app: &mut AppBuilder) {
            app.insert_resource(42_i32);
        }
    }

    #[test]
    fn plugin_name正确() {
        let plugin = TestPlugin;
        assert_eq!(plugin.name(), "test_plugin");
    }

    #[test]
    fn 默认无依赖() {
        let plugin = TestPlugin;
        assert!(plugin.dependencies().is_empty());
    }

    #[test]
    fn plugin_build注册资源() {
        let plugin = TestPlugin;
        let mut app = AppBuilder::new();
        plugin.build(&mut app);
        assert_eq!(*app.resources.get::<i32>().unwrap(), 42);
    }

    struct PluginWithDeps;
    impl Plugin for PluginWithDeps {
        fn name(&self) -> &str {
            "with_deps"
        }

        fn dependencies(&self) -> Vec<&str> {
            vec!["test_plugin"]
        }

        fn build(&self, app: &mut AppBuilder) {
            app.insert_resource(true);
        }
    }

    #[test]
    fn 插件可以声明依赖() {
        let plugin = PluginWithDeps;
        let deps = plugin.dependencies();
        assert_eq!(deps, vec!["test_plugin"]);
    }
}
