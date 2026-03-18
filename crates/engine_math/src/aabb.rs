// 轴对齐包围盒 (AABB)

use glam::Vec3;

/// 轴对齐包围盒
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct AABB {
    pub min: Vec3,
    pub max: Vec3,
}

impl AABB {
    pub fn new(min: Vec3, max: Vec3) -> Self {
        Self {
            min: min.min(max),
            max: min.max(max),
        }
    }

    pub fn center(&self) -> Vec3 {
        (self.min + self.max) * 0.5
    }

    pub fn size(&self) -> Vec3 {
        self.max - self.min
    }

    pub fn contains(&self, point: Vec3) -> bool {
        point.x >= self.min.x
            && point.x <= self.max.x
            && point.y >= self.min.y
            && point.y <= self.max.y
            && point.z >= self.min.z
            && point.z <= self.max.z
    }

    pub fn intersects(&self, other: &AABB) -> bool {
        self.min.x <= other.max.x
            && self.max.x >= other.min.x
            && self.min.y <= other.max.y
            && self.max.y >= other.min.y
            && self.min.z <= other.max.z
            && self.max.z >= other.min.z
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn new_creates_aabb_with_min_max() {
        let aabb = AABB::new(Vec3::ZERO, Vec3::ONE);
        assert_eq!(aabb.min, Vec3::ZERO);
        assert_eq!(aabb.max, Vec3::ONE);
    }

    #[test]
    fn new_auto_corrects_swapped_min_max() {
        // 当 min > max 时，构造函数应自动修正
        let aabb = AABB::new(Vec3::ONE, Vec3::ZERO);
        assert_eq!(aabb.min, Vec3::ZERO);
        assert_eq!(aabb.max, Vec3::ONE);
    }

    #[test]
    fn center_returns_midpoint() {
        let aabb = AABB::new(Vec3::ZERO, Vec3::new(2.0, 4.0, 6.0));
        assert_eq!(aabb.center(), Vec3::new(1.0, 2.0, 3.0));
    }

    #[test]
    fn size_returns_dimensions() {
        let aabb = AABB::new(Vec3::new(1.0, 2.0, 3.0), Vec3::new(4.0, 6.0, 9.0));
        assert_eq!(aabb.size(), Vec3::new(3.0, 4.0, 6.0));
    }

    #[test]
    fn contains_point_inside() {
        let aabb = AABB::new(Vec3::ZERO, Vec3::ONE);
        assert!(aabb.contains(Vec3::splat(0.5)));
    }

    #[test]
    fn contains_point_on_boundary() {
        // 边界上的点应被包含
        let aabb = AABB::new(Vec3::ZERO, Vec3::ONE);
        assert!(aabb.contains(Vec3::ZERO));
        assert!(aabb.contains(Vec3::ONE));
    }

    #[test]
    fn does_not_contain_point_outside() {
        let aabb = AABB::new(Vec3::ZERO, Vec3::ONE);
        assert!(!aabb.contains(Vec3::new(2.0, 0.5, 0.5)));
        assert!(!aabb.contains(Vec3::new(-1.0, 0.5, 0.5)));
        assert!(!aabb.contains(Vec3::new(0.5, 0.5, 1.1)));
    }

    #[test]
    fn intersects_overlapping_aabbs() {
        let a = AABB::new(Vec3::ZERO, Vec3::new(2.0, 2.0, 2.0));
        let b = AABB::new(Vec3::ONE, Vec3::new(3.0, 3.0, 3.0));
        assert!(a.intersects(&b));
        assert!(b.intersects(&a));
    }

    #[test]
    fn intersects_touching_aabbs() {
        // 仅边缘接触的 AABB 也算相交
        let a = AABB::new(Vec3::ZERO, Vec3::ONE);
        let b = AABB::new(Vec3::ONE, Vec3::new(2.0, 2.0, 2.0));
        assert!(a.intersects(&b));
    }

    #[test]
    fn does_not_intersect_separated_aabbs() {
        let a = AABB::new(Vec3::ZERO, Vec3::ONE);
        let b = AABB::new(Vec3::new(5.0, 5.0, 5.0), Vec3::new(6.0, 6.0, 6.0));
        assert!(!a.intersects(&b));
        assert!(!b.intersects(&a));
    }

    #[test]
    fn does_not_intersect_on_single_axis_gap() {
        // 只有一个轴分离就不相交
        let a = AABB::new(Vec3::ZERO, Vec3::ONE);
        let b = AABB::new(Vec3::new(0.0, 0.0, 2.0), Vec3::new(1.0, 1.0, 3.0));
        assert!(!a.intersects(&b));
    }

    #[test]
    fn degenerate_aabb_zero_volume() {
        // 退化的零体积 AABB（一个平面）
        let aabb = AABB::new(Vec3::ZERO, Vec3::new(1.0, 1.0, 0.0));
        assert_eq!(aabb.size(), Vec3::new(1.0, 1.0, 0.0));
        assert!(aabb.contains(Vec3::new(0.5, 0.5, 0.0)));
    }
}
