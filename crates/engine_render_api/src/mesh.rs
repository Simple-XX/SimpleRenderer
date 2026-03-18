// 网格数据定义

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct MeshHandle(pub u64);

pub struct MeshData {
    pub positions: Vec<glam::Vec3>,
    pub normals: Vec<glam::Vec3>,
    pub tex_coords: Vec<glam::Vec2>,
    pub indices: Vec<u32>,
}
