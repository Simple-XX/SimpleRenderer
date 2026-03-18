// 射线与 AABB 相交测试

use glam::Vec3;

use crate::aabb::AABB;

/// 射线: origin + t * direction
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Ray {
    pub origin: Vec3,
    pub direction: Vec3,
}

impl Ray {
    pub fn new(origin: Vec3, direction: Vec3) -> Self {
        Self {
            origin,
            direction: direction.normalize(),
        }
    }

    pub fn point_at(&self, t: f32) -> Vec3 {
        self.origin + self.direction * t
    }

    // Slab 方法: 计算射线与 AABB 的交点参数 t
    pub fn intersects_aabb(&self, aabb: &AABB) -> Option<f32> {
        let inv_dir = 1.0 / self.direction;

        let t1 = (aabb.min - self.origin) * inv_dir;
        let t2 = (aabb.max - self.origin) * inv_dir;

        let t_min_v = t1.min(t2);
        let t_max_v = t1.max(t2);

        let t_enter = t_min_v.x.max(t_min_v.y).max(t_min_v.z);
        let t_exit = t_max_v.x.min(t_max_v.y).min(t_max_v.z);

        if t_enter <= t_exit {
            Some(t_enter)
        } else {
            None
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn new_creates_ray() {
        let ray = Ray::new(Vec3::ZERO, Vec3::X);
        assert_eq!(ray.origin, Vec3::ZERO);
        // 方向应被归一化
        assert!((ray.direction - Vec3::X).length() < 1e-6);
    }

    #[test]
    fn new_normalizes_direction() {
        let ray = Ray::new(Vec3::ZERO, Vec3::new(3.0, 0.0, 0.0));
        assert!((ray.direction.length() - 1.0).abs() < 1e-6);
        assert!((ray.direction - Vec3::X).length() < 1e-6);
    }

    #[test]
    fn point_at_returns_position_along_ray() {
        let ray = Ray::new(Vec3::ZERO, Vec3::X);
        let p = ray.point_at(5.0);
        assert!((p - Vec3::new(5.0, 0.0, 0.0)).length() < 1e-6);
    }

    #[test]
    fn point_at_negative_t() {
        // 负 t 值返回射线反方向的点
        let ray = Ray::new(Vec3::new(1.0, 0.0, 0.0), Vec3::X);
        let p = ray.point_at(-1.0);
        assert!((p - Vec3::ZERO).length() < 1e-6);
    }

    #[test]
    fn intersects_aabb_hit() {
        let ray = Ray::new(Vec3::new(-5.0, 0.5, 0.5), Vec3::X);
        let aabb = AABB::new(Vec3::ZERO, Vec3::ONE);
        let t = ray.intersects_aabb(&aabb);
        assert!(t.is_some());
        let t = t.unwrap();
        assert!((t - 5.0).abs() < 1e-4);
    }

    #[test]
    fn intersects_aabb_miss() {
        // 射线与 AABB 完全不相交
        let ray = Ray::new(Vec3::new(-5.0, 5.0, 5.0), Vec3::X);
        let aabb = AABB::new(Vec3::ZERO, Vec3::ONE);
        assert!(ray.intersects_aabb(&aabb).is_none());
    }

    #[test]
    fn intersects_aabb_ray_inside() {
        // 射线起点在 AABB 内部，t 应 <= 0（最近交点在身后）
        // 返回的是进入点的 t，可能为负
        let ray = Ray::new(Vec3::splat(0.5), Vec3::X);
        let aabb = AABB::new(Vec3::ZERO, Vec3::ONE);
        let t = ray.intersects_aabb(&aabb);
        // 应返回 Some，因为射线确实穿过 AABB
        assert!(t.is_some());
    }

    #[test]
    fn intersects_aabb_along_y_axis() {
        let ray = Ray::new(Vec3::new(0.5, -3.0, 0.5), Vec3::Y);
        let aabb = AABB::new(Vec3::ZERO, Vec3::ONE);
        let t = ray.intersects_aabb(&aabb);
        assert!(t.is_some());
        assert!((t.unwrap() - 3.0).abs() < 1e-4);
    }

    #[test]
    fn intersects_aabb_along_z_axis() {
        let ray = Ray::new(Vec3::new(0.5, 0.5, -2.0), Vec3::Z);
        let aabb = AABB::new(Vec3::ZERO, Vec3::ONE);
        let t = ray.intersects_aabb(&aabb);
        assert!(t.is_some());
        assert!((t.unwrap() - 2.0).abs() < 1e-4);
    }

    #[test]
    fn intersects_aabb_diagonal_ray() {
        // 对角线方向的射线
        let ray = Ray::new(Vec3::new(-1.0, -1.0, -1.0), Vec3::new(1.0, 1.0, 1.0));
        let aabb = AABB::new(Vec3::ZERO, Vec3::ONE);
        let t = ray.intersects_aabb(&aabb);
        assert!(t.is_some());
    }

    #[test]
    fn intersects_aabb_parallel_miss() {
        // 射线平行于 AABB 的一个面但不在其内
        let ray = Ray::new(Vec3::new(-1.0, 2.0, 0.5), Vec3::X);
        let aabb = AABB::new(Vec3::ZERO, Vec3::ONE);
        assert!(ray.intersects_aabb(&aabb).is_none());
    }
}
