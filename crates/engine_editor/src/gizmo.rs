/// 当前 Gizmo 工具模式
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum GizmoTool {
    /// 仅选择，不显示 gizmo
    #[default]
    Select,
    /// 显示移动 gizmo（XYZ 箭头）
    Translate,
    /// 显示旋转 gizmo（XYZ 圆环）
    Rotate,
    /// 显示缩放 gizmo（XYZ 方块）
    Scale,
}

impl GizmoTool {
    pub fn shortcut(&self) -> &'static str {
        match self {
            GizmoTool::Select => "Q",
            GizmoTool::Translate => "W",
            GizmoTool::Rotate => "E",
            GizmoTool::Scale => "R",
        }
    }
}

/// 坐标系
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum CoordinateSpace {
    /// Gizmo 沿实体自身坐标轴
    #[default]
    Local,
    /// Gizmo 沿世界 XYZ 轴
    World,
}

impl CoordinateSpace {}

/// Gizmo 全局状态
#[derive(Debug, Default)]
pub struct GizmoState {
    /// 当前工具
    pub tool: GizmoTool,
    /// 当前坐标系
    pub space: CoordinateSpace,
    /// 是否启用吸附
    pub snap_enabled: bool,
    /// 移动吸附间距
    pub snap_translate: f32,
    /// 旋转吸附角度（度）
    pub snap_rotate: f32,
    /// 缩放吸附间距
    pub snap_scale: f32,
}

impl GizmoState {
    pub fn new() -> Self {
        Self {
            snap_translate: 0.5,
            snap_rotate: 15.0,
            snap_scale: 0.1,
            ..Default::default()
        }
    }

    /// 切换 Local/World 坐标系
    pub fn toggle_space(&mut self) {
        self.space = match self.space {
            CoordinateSpace::Local => CoordinateSpace::World,
            CoordinateSpace::World => CoordinateSpace::Local,
        };
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn default_gizmo_state() {
        let state = GizmoState::new();
        assert_eq!(state.tool, GizmoTool::Select);
        assert_eq!(state.space, CoordinateSpace::Local);
        assert!(!state.snap_enabled);
    }

    #[test]
    fn toggle_space() {
        let mut state = GizmoState::new();
        assert_eq!(state.space, CoordinateSpace::Local);
        state.toggle_space();
        assert_eq!(state.space, CoordinateSpace::World);
        state.toggle_space();
        assert_eq!(state.space, CoordinateSpace::Local);
    }

    #[test]
    fn gizmo_tool_shortcut() {
        assert_eq!(GizmoTool::Select.shortcut(), "Q");
        assert_eq!(GizmoTool::Translate.shortcut(), "W");
        assert_eq!(GizmoTool::Rotate.shortcut(), "E");
        assert_eq!(GizmoTool::Scale.shortcut(), "R");
    }
}
