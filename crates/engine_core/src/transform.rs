//! 变换组件 — Transform、Parent、Children、GlobalTransform。
//!
//! 提供基础的空间变换（位置、旋转、缩放）和层级关系。

use glam::{Mat4, Quat, Vec3};

/// 局部变换组件，包含位置、旋转和缩放。
#[derive(Debug, Clone)]
pub struct Transform {
    /// 平移（位置）
    pub translation: Vec3,
    /// 旋转（四元数）
    pub rotation: Quat,
    /// 缩放
    pub scale: Vec3,
}

impl Default for Transform {
    fn default() -> Self {
        Self {
            translation: Vec3::ZERO,
            rotation: Quat::IDENTITY,
            scale: Vec3::ONE,
        }
    }
}

impl Transform {
    /// 从平移创建变换（旋转为单位四元数，缩放为 1）。
    pub fn from_translation(translation: Vec3) -> Self {
        Self {
            translation,
            ..Default::default()
        }
    }

    /// 从旋转创建变换（位置为原点，缩放为 1）。
    pub fn from_rotation(rotation: Quat) -> Self {
        Self {
            rotation,
            ..Default::default()
        }
    }

    /// 从缩放创建变换（位置为原点，旋转为单位四元数）。
    pub fn from_scale(scale: Vec3) -> Self {
        Self {
            scale,
            ..Default::default()
        }
    }

    /// 转换为 4x4 变换矩阵。
    pub fn to_mat4(&self) -> Mat4 {
        Mat4::from_scale_rotation_translation(self.scale, self.rotation, self.translation)
    }
}

/// 父实体组件，存储父实体的引用。
pub struct Parent(pub hecs::Entity);

/// 子实体列表组件。
pub struct Children(pub Vec<hecs::Entity>);

/// 全局变换组件，存储世界空间中的最终变换矩阵。
pub struct GlobalTransform(pub Mat4);

impl Default for GlobalTransform {
    fn default() -> Self {
        Self(Mat4::IDENTITY)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn 默认变换为单位变换() {
        let t = Transform::default();
        assert_eq!(t.translation, Vec3::ZERO);
        assert_eq!(t.rotation, Quat::IDENTITY);
        assert_eq!(t.scale, Vec3::ONE);
    }

    #[test]
    fn from_translation创建正确变换() {
        let t = Transform::from_translation(Vec3::new(1.0, 2.0, 3.0));
        assert_eq!(t.translation, Vec3::new(1.0, 2.0, 3.0));
        assert_eq!(t.rotation, Quat::IDENTITY);
        assert_eq!(t.scale, Vec3::ONE);
    }

    #[test]
    fn from_rotation创建正确变换() {
        let rot = Quat::from_rotation_z(std::f32::consts::FRAC_PI_2);
        let t = Transform::from_rotation(rot);
        assert_eq!(t.translation, Vec3::ZERO);
        assert!((t.rotation.x - rot.x).abs() < f32::EPSILON);
        assert_eq!(t.scale, Vec3::ONE);
    }

    #[test]
    fn from_scale创建正确变换() {
        let t = Transform::from_scale(Vec3::new(2.0, 3.0, 4.0));
        assert_eq!(t.translation, Vec3::ZERO);
        assert_eq!(t.rotation, Quat::IDENTITY);
        assert_eq!(t.scale, Vec3::new(2.0, 3.0, 4.0));
    }

    #[test]
    fn to_mat4默认变换等于单位矩阵() {
        let t = Transform::default();
        let mat = t.to_mat4();
        assert_eq!(mat, Mat4::IDENTITY);
    }

    #[test]
    fn to_mat4平移正确() {
        let t = Transform::from_translation(Vec3::new(5.0, 0.0, 0.0));
        let mat = t.to_mat4();
        // 矩阵第四列存储平移
        let col3 = mat.col(3);
        assert!((col3.x - 5.0).abs() < f32::EPSILON);
        assert!((col3.y).abs() < f32::EPSILON);
        assert!((col3.z).abs() < f32::EPSILON);
    }

    #[test]
    fn global_transform默认为单位矩阵() {
        let gt = GlobalTransform::default();
        assert_eq!(gt.0, Mat4::IDENTITY);
    }

    #[test]
    fn parent_children与hecs集成() {
        let mut world = hecs::World::new();
        let parent_entity = world.spawn((Transform::default(),));
        let child_entity = world.spawn((
            Transform::from_translation(Vec3::new(1.0, 0.0, 0.0)),
            Parent(parent_entity),
        ));

        // 给父实体添加 Children 组件
        world
            .insert_one(parent_entity, Children(vec![child_entity]))
            .unwrap();

        // 验证关系
        let parent_comp = world.get::<&Parent>(child_entity).unwrap();
        assert_eq!(parent_comp.0, parent_entity);

        let children_comp = world.get::<&Children>(parent_entity).unwrap();
        assert_eq!(children_comp.0.len(), 1);
        assert_eq!(children_comp.0[0], child_entity);
    }
}
