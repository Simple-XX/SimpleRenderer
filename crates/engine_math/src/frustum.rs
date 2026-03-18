// 视锥体 (View Frustum) 裁剪

use glam::{Mat4, Vec3, Vec4};

use crate::aabb::AABB;

/// 平面方程: normal · point + distance = 0
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Plane {
    pub normal: Vec3,
    pub distance: f32,
}

impl Plane {
    pub fn distance_to_point(&self, point: Vec3) -> f32 {
        self.normal.dot(point) + self.distance
    }

    fn from_vec4(v: Vec4) -> Self {
        let len = Vec3::new(v.x, v.y, v.z).length();
        Self {
            normal: Vec3::new(v.x, v.y, v.z) / len,
            distance: v.w / len,
        }
    }
}

/// 视锥体: 6 个裁剪平面（近、远、左、右、上、下）
#[derive(Debug, Clone, Copy)]
pub struct Frustum {
    pub planes: [Plane; 6],
}

impl Frustum {
    // Gribb-Hartmann 方法: 从 view-projection 矩阵提取 6 个裁剪平面
    pub fn from_view_projection(vp: &Mat4) -> Self {
        let row0 = vp.row(0);
        let row1 = vp.row(1);
        let row2 = vp.row(2);
        let row3 = vp.row(3);

        let planes = [
            Plane::from_vec4(row3 + row0), // 左
            Plane::from_vec4(row3 - row0), // 右
            Plane::from_vec4(row3 + row1), // 下
            Plane::from_vec4(row3 - row1), // 上
            Plane::from_vec4(row3 + row2), // 近
            Plane::from_vec4(row3 - row2), // 远
        ];

        Self { planes }
    }

    pub fn contains_point(&self, point: Vec3) -> bool {
        self.planes
            .iter()
            .all(|plane| plane.distance_to_point(point) >= 0.0)
    }

    pub fn intersects_aabb(&self, aabb: &AABB) -> bool {
        for plane in &self.planes {
            // 找到 AABB 上离平面法线方向最远的顶点 (p-vertex)
            let p = Vec3::new(
                if plane.normal.x >= 0.0 {
                    aabb.max.x
                } else {
                    aabb.min.x
                },
                if plane.normal.y >= 0.0 {
                    aabb.max.y
                } else {
                    aabb.min.y
                },
                if plane.normal.z >= 0.0 {
                    aabb.max.z
                } else {
                    aabb.min.z
                },
            );

            if plane.distance_to_point(p) < 0.0 {
                return false;
            }
        }
        true
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // ---- Plane 测试 ----

    #[test]
    fn plane_distance_to_point_on_plane() {
        // 点在平面上，距离为 0
        let plane = Plane {
            normal: Vec3::Y,
            distance: 0.0,
        };
        assert!((plane.distance_to_point(Vec3::ZERO)).abs() < 1e-6);
        assert!((plane.distance_to_point(Vec3::X)).abs() < 1e-6);
    }

    #[test]
    fn plane_distance_to_point_positive_side() {
        // 法线方向一侧为正
        let plane = Plane {
            normal: Vec3::Y,
            distance: 0.0,
        };
        let d = plane.distance_to_point(Vec3::new(0.0, 5.0, 0.0));
        assert!((d - 5.0).abs() < 1e-6);
    }

    #[test]
    fn plane_distance_to_point_negative_side() {
        let plane = Plane {
            normal: Vec3::Y,
            distance: 0.0,
        };
        let d = plane.distance_to_point(Vec3::new(0.0, -3.0, 0.0));
        assert!((d - (-3.0)).abs() < 1e-6);
    }

    #[test]
    fn plane_distance_with_offset() {
        // distance = -2 表示平面在 y=2 处
        let plane = Plane {
            normal: Vec3::Y,
            distance: -2.0,
        };
        let d = plane.distance_to_point(Vec3::new(0.0, 2.0, 0.0));
        assert!(d.abs() < 1e-6);
    }

    // ---- Frustum 测试 ----

    fn make_test_frustum() -> Frustum {
        // 使用标准透视投影矩阵
        let proj = Mat4::perspective_rh(
            std::f32::consts::FRAC_PI_4, // 45° FOV
            1.0,                          // 宽高比
            0.1,                          // 近平面
            100.0,                        // 远平面
        );
        // 相机在原点，看向 -Z
        let view = Mat4::look_at_rh(Vec3::ZERO, Vec3::NEG_Z, Vec3::Y);
        Frustum::from_view_projection(&(proj * view))
    }

    #[test]
    fn frustum_from_view_projection_produces_6_planes() {
        let frustum = make_test_frustum();
        assert_eq!(frustum.planes.len(), 6);
    }

    #[test]
    fn frustum_contains_point_at_center() {
        let frustum = make_test_frustum();
        // 视锥体中心附近的点（在 -Z 方向）
        assert!(frustum.contains_point(Vec3::new(0.0, 0.0, -10.0)));
    }

    #[test]
    fn frustum_does_not_contain_point_behind_camera() {
        let frustum = make_test_frustum();
        // 相机后方的点
        assert!(!frustum.contains_point(Vec3::new(0.0, 0.0, 10.0)));
    }

    #[test]
    fn frustum_does_not_contain_point_far_outside() {
        let frustum = make_test_frustum();
        assert!(!frustum.contains_point(Vec3::new(1000.0, 0.0, -10.0)));
    }

    #[test]
    fn frustum_does_not_contain_point_beyond_far_plane() {
        let frustum = make_test_frustum();
        // 远平面之外的点
        assert!(!frustum.contains_point(Vec3::new(0.0, 0.0, -200.0)));
    }

    #[test]
    fn frustum_does_not_contain_point_before_near_plane() {
        let frustum = make_test_frustum();
        // 近平面之前（但仍在相机前方）
        assert!(!frustum.contains_point(Vec3::new(0.0, 0.0, -0.01)));
    }

    #[test]
    fn frustum_intersects_aabb_inside() {
        let frustum = make_test_frustum();
        // 完全在视锥体内部的 AABB
        let aabb = AABB::new(Vec3::new(-1.0, -1.0, -11.0), Vec3::new(1.0, 1.0, -9.0));
        assert!(frustum.intersects_aabb(&aabb));
    }

    #[test]
    fn frustum_intersects_aabb_partially_inside() {
        let frustum = make_test_frustum();
        // 部分在视锥体内的 AABB（跨越近平面）
        let aabb = AABB::new(Vec3::new(-0.5, -0.5, -1.0), Vec3::new(0.5, 0.5, 1.0));
        assert!(frustum.intersects_aabb(&aabb));
    }

    #[test]
    fn frustum_does_not_intersect_aabb_behind_camera() {
        let frustum = make_test_frustum();
        // 完全在相机后方的 AABB
        let aabb = AABB::new(Vec3::new(-1.0, -1.0, 5.0), Vec3::new(1.0, 1.0, 10.0));
        assert!(!frustum.intersects_aabb(&aabb));
    }

    #[test]
    fn frustum_does_not_intersect_aabb_beyond_far_plane() {
        let frustum = make_test_frustum();
        // 远平面之外的 AABB
        let aabb = AABB::new(
            Vec3::new(-1.0, -1.0, -200.0),
            Vec3::new(1.0, 1.0, -150.0),
        );
        assert!(!frustum.intersects_aabb(&aabb));
    }

    #[test]
    fn frustum_does_not_intersect_aabb_far_to_side() {
        let frustum = make_test_frustum();
        // 视锥体侧面外的 AABB
        let aabb = AABB::new(
            Vec3::new(100.0, 100.0, -10.0),
            Vec3::new(101.0, 101.0, -9.0),
        );
        assert!(!frustum.intersects_aabb(&aabb));
    }

    #[test]
    fn frustum_with_orthographic_projection() {
        // 正交投影的视锥体是一个长方体
        let proj = Mat4::orthographic_rh(-10.0, 10.0, -10.0, 10.0, 0.1, 100.0);
        let view = Mat4::look_at_rh(Vec3::ZERO, Vec3::NEG_Z, Vec3::Y);
        let frustum = Frustum::from_view_projection(&(proj * view));

        assert!(frustum.contains_point(Vec3::new(0.0, 0.0, -50.0)));
        assert!(!frustum.contains_point(Vec3::new(20.0, 0.0, -50.0)));
        assert!(!frustum.contains_point(Vec3::new(0.0, 0.0, -200.0)));
    }
}
