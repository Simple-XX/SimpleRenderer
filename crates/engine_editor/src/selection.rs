use engine_core::{Entity, Transform};
use engine_math::{Ray, AABB};
use engine_scene::{BoundingBox, Scene};
use glam::{Mat4, Vec3};

/// 编辑器选择状态
#[derive(Debug, Default)]
pub struct SelectionState {
    /// 当前选中的实体
    pub selected: Option<Entity>,
    /// 上一次选中的实体（用于双击等操作）
    pub previous: Option<Entity>,
}

impl SelectionState {
    /// 选中一个实体
    pub fn select(&mut self, entity: Entity) {
        self.previous = self.selected;
        self.selected = Some(entity);
    }

    /// 取消选择
    pub fn deselect(&mut self) {
        self.previous = self.selected;
        self.selected = None;
    }

    /// 清除选择（等同于 deselect）
    pub fn clear(&mut self) {
        self.deselect();
    }

    /// 检查实体是否被选中
    pub fn is_selected(&self, entity: Entity) -> bool {
        self.selected == Some(entity)
    }
}

/// 从屏幕坐标生成世界空间射线
pub fn screen_to_ray(
    screen_x: f32,
    screen_y: f32,
    viewport_rect: (f32, f32, f32, f32), // (x, y, width, height)
    view_matrix: Mat4,
    projection_matrix: Mat4,
) -> Ray {
    let (vx, vy, vw, vh) = viewport_rect;
    // 归一化设备坐标
    let ndc_x = ((screen_x - vx) / vw) * 2.0 - 1.0;
    let ndc_y = 1.0 - ((screen_y - vy) / vh) * 2.0;

    let inv_proj = projection_matrix.inverse();
    let inv_view = view_matrix.inverse();

    let clip_near = glam::Vec4::new(ndc_x, ndc_y, -1.0, 1.0);
    let clip_far = glam::Vec4::new(ndc_x, ndc_y, 1.0, 1.0);

    let eye_near = inv_proj * clip_near;
    let eye_near = eye_near / eye_near.w;
    let eye_far = inv_proj * clip_far;
    let eye_far = eye_far / eye_far.w;

    let world_near = inv_view * eye_near;
    let world_far = inv_view * eye_far;

    let origin = Vec3::new(world_near.x, world_near.y, world_near.z);
    let direction = Vec3::new(
        world_far.x - world_near.x,
        world_far.y - world_near.y,
        world_far.z - world_near.z,
    );

    Ray::new(origin, direction)
}

/// 在场景中拾取实体（射线与包围盒相交测试）
pub fn pick_entity(ray: &Ray, scene: &Scene) -> Option<Entity> {
    let mut closest: Option<(Entity, f32)> = None;

    for (entity, (transform, bbox)) in scene.world.query::<(&Transform, &BoundingBox)>().iter() {
        // 将 AABB 变换到世界空间（简单平移）
        let world_aabb = AABB::new(
            bbox.aabb.min + transform.translation,
            bbox.aabb.max + transform.translation,
        );

        if let Some(t) = ray.intersects_aabb(&world_aabb) {
            if t >= 0.0 {
                match closest {
                    Some((_, best_t)) if t < best_t => closest = Some((entity, t)),
                    None => closest = Some((entity, t)),
                    _ => {}
                }
            }
        }
    }

    closest.map(|(e, _)| e)
}

#[cfg(test)]
mod tests {
    use super::*;
    use engine_core::World;
    use glam::Vec3;

    #[test]
    fn select_and_deselect() {
        let mut world = World::new();
        let e = world.spawn(());
        let mut sel = SelectionState::default();
        assert!(sel.selected.is_none());

        sel.select(e);
        assert!(sel.is_selected(e));

        sel.deselect();
        assert!(sel.selected.is_none());
        assert_eq!(sel.previous, Some(e));
    }

    #[test]
    fn select_replaces_previous() {
        let mut world = World::new();
        let e1 = world.spawn(());
        let e2 = world.spawn(());
        let mut sel = SelectionState::default();

        sel.select(e1);
        sel.select(e2);
        assert!(sel.is_selected(e2));
        assert!(!sel.is_selected(e1));
        assert_eq!(sel.previous, Some(e1));
    }

    #[test]
    fn screen_to_ray_center_of_viewport() {
        let view = Mat4::look_at_rh(Vec3::new(0.0, 0.0, 3.0), Vec3::ZERO, Vec3::Y);
        let proj = Mat4::perspective_rh_gl(60.0_f32.to_radians(), 1.0, 0.1, 100.0);
        let ray = screen_to_ray(400.0, 300.0, (0.0, 0.0, 800.0, 600.0), view, proj);
        assert!(ray.direction.z < 0.0);
    }

    #[test]
    fn pick_entity_finds_closest() {
        let mut scene = engine_scene::Scene::new();
        let e1 = scene.world.spawn((
            engine_scene::Name("A".into()),
            Transform::from_translation(Vec3::new(0.0, 0.0, -2.0)),
            BoundingBox::default(),
        ));
        let _e2 = scene.world.spawn((
            engine_scene::Name("B".into()),
            Transform::from_translation(Vec3::new(0.0, 0.0, -5.0)),
            BoundingBox::default(),
        ));
        let ray = Ray::new(Vec3::new(0.0, 0.0, 0.0), Vec3::new(0.0, 0.0, -1.0));
        let picked = pick_entity(&ray, &scene);
        assert_eq!(picked, Some(e1));
    }

    #[test]
    fn pick_entity_misses_when_no_bbox() {
        let mut scene = engine_scene::Scene::new();
        scene
            .world
            .spawn((engine_scene::Name("NoBBox".into()), Transform::default()));
        let ray = Ray::new(Vec3::ZERO, Vec3::NEG_Z);
        assert!(pick_entity(&ray, &scene).is_none());
    }
}
