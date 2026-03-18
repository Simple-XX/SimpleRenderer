//! 项目管理 — 创建、打开、保存项目，以及导入资产。

use std::fs;
use std::path::{Path, PathBuf};

use serde::{Deserialize, Serialize};

const PROJECT_FILE_NAME: &str = "project.toml";
const ASSETS_DIR: &str = "assets";
const MODELS_DIR: &str = "models";
const TEXTURES_DIR: &str = "textures";

#[derive(Debug, Serialize, Deserialize)]
pub struct Project {
    pub meta: ProjectMeta,
    #[serde(default)]
    pub scene: SceneConfig,
    #[serde(default)]
    pub assets: AssetRegistry,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct ProjectMeta {
    pub name: String,
    pub engine_version: String,
}

#[derive(Debug, Default, Serialize, Deserialize)]
pub struct SceneConfig {
    #[serde(default)]
    pub camera: CameraConfig,
    #[serde(default)]
    pub light: LightConfig,
    #[serde(default)]
    pub render: RenderConfig,
    #[serde(default)]
    pub layout: EditorLayout,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct CameraConfig {
    pub position: [f32; 3],
    pub yaw: f32,
    pub pitch: f32,
    pub fov: f32,
    pub speed: f32,
}

impl Default for CameraConfig {
    fn default() -> Self {
        Self {
            position: [0.0, 0.0, 3.0],
            yaw: -90.0,
            pitch: 0.0,
            fov: 60.0,
            speed: 2.5,
        }
    }
}

#[derive(Debug, Serialize, Deserialize)]
pub struct LightConfig {
    pub direction: [f32; 3],
    pub color: [u8; 4],
    pub intensity: f32,
}

impl Default for LightConfig {
    fn default() -> Self {
        Self {
            direction: [1.0, 5.0, 1.0],
            color: [255, 255, 255, 255],
            intensity: 1.0,
        }
    }
}

#[derive(Debug, Serialize, Deserialize)]
pub struct RenderConfig {
    pub mode: String,
    pub tile_size: usize,
    pub early_z: bool,
    pub shininess: f32,
}

impl Default for RenderConfig {
    fn default() -> Self {
        Self {
            mode: "TileBased".to_string(),
            tile_size: 64,
            early_z: true,
            shininess: 32.0,
        }
    }
}

#[derive(Debug, Serialize, Deserialize)]
pub struct EditorLayout {
    pub scene_tree_width: f32,
    pub properties_width: f32,
    pub console_height: f32,
}

impl Default for EditorLayout {
    fn default() -> Self {
        Self {
            scene_tree_width: 200.0,
            properties_width: 280.0,
            console_height: 150.0,
        }
    }
}

#[derive(Debug, Default, Serialize, Deserialize)]
pub struct AssetRegistry {
    #[serde(default)]
    pub models: Vec<AssetEntry>,
    #[serde(default)]
    pub textures: Vec<AssetEntry>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AssetEntry {
    pub name: String,
    /// 项目内的相对路径
    pub path: String,
}

#[derive(Debug)]
pub struct ProjectState {
    pub project: Project,
    pub project_dir: PathBuf,
    pub dirty: bool,
}

impl Project {
    pub fn new(name: &str) -> Self {
        Self {
            meta: ProjectMeta {
                name: name.to_string(),
                engine_version: env!("CARGO_PKG_VERSION").to_string(),
            },
            scene: SceneConfig::default(),
            assets: AssetRegistry::default(),
        }
    }
}

impl ProjectState {
    /// 创建新项目：在指定目录中创建项目结构并写入 project.toml。
    pub fn create(project_dir: &Path, name: &str) -> Result<Self, ProjectError> {
        if project_dir.join(PROJECT_FILE_NAME).exists() {
            return Err(ProjectError::AlreadyExists(project_dir.to_path_buf()));
        }

        fs::create_dir_all(project_dir)?;
        fs::create_dir_all(project_dir.join(ASSETS_DIR).join(MODELS_DIR))?;
        fs::create_dir_all(project_dir.join(ASSETS_DIR).join(TEXTURES_DIR))?;

        let project = Project::new(name);
        let state = Self {
            project,
            project_dir: project_dir.to_path_buf(),
            dirty: false,
        };
        state.save()?;

        tracing::info!("项目已创建: {}", project_dir.display());
        Ok(state)
    }

    /// 打开已有项目：读取 project.toml 并解析。
    pub fn open(project_dir: &Path) -> Result<Self, ProjectError> {
        let config_path = project_dir.join(PROJECT_FILE_NAME);
        if !config_path.exists() {
            return Err(ProjectError::NotFound(config_path));
        }

        let content = fs::read_to_string(&config_path)?;
        let project: Project =
            toml::from_str(&content).map_err(|e| ProjectError::ParseError(e.to_string()))?;

        tracing::info!(
            "项目已打开: {} ({})",
            project.meta.name,
            project_dir.display()
        );
        Ok(Self {
            project,
            project_dir: project_dir.to_path_buf(),
            dirty: false,
        })
    }

    /// 保存项目：将当前配置序列化为 TOML 并写入 project.toml。
    pub fn save(&self) -> Result<(), ProjectError> {
        let content = toml::to_string_pretty(&self.project)
            .map_err(|e| ProjectError::SerializeError(e.to_string()))?;
        let config_path = self.project_dir.join(PROJECT_FILE_NAME);
        fs::write(&config_path, content)?;

        tracing::info!("项目已保存: {}", config_path.display());
        Ok(())
    }

    /// 导入模型文件到项目的 assets/models/ 目录。
    pub fn import_model(&mut self, source: &Path) -> Result<AssetEntry, ProjectError> {
        let file_name = source
            .file_name()
            .ok_or_else(|| ProjectError::InvalidPath(source.to_path_buf()))?;

        let dest_dir = self.project_dir.join(ASSETS_DIR).join(MODELS_DIR);
        fs::create_dir_all(&dest_dir)?;
        let dest = dest_dir.join(file_name);

        // 复制主模型文件
        fs::copy(source, &dest)?;

        // 如果是 .obj 文件，尝试同时复制同目录下的 .mtl 文件和纹理
        if let Some(ext) = source.extension() {
            if ext.eq_ignore_ascii_case("obj") {
                self.copy_obj_companions(source, &dest_dir);
            }
        }

        let rel_path = format!(
            "{}/{}/{}",
            ASSETS_DIR,
            MODELS_DIR,
            file_name.to_string_lossy()
        );

        let name = source
            .file_stem()
            .map(|s| s.to_string_lossy().to_string())
            .unwrap_or_else(|| "model".to_string());

        let entry = AssetEntry {
            name: name.clone(),
            path: rel_path,
        };

        // 避免重复添加
        if !self.project.assets.models.iter().any(|e| e.name == name) {
            self.project.assets.models.push(entry.clone());
        }

        self.dirty = true;
        tracing::info!("模型已导入: {}", source.display());
        Ok(entry)
    }

    /// 导入纹理文件到项目的 assets/textures/ 目录。
    pub fn import_texture(&mut self, source: &Path) -> Result<AssetEntry, ProjectError> {
        let file_name = source
            .file_name()
            .ok_or_else(|| ProjectError::InvalidPath(source.to_path_buf()))?;

        let dest_dir = self.project_dir.join(ASSETS_DIR).join(TEXTURES_DIR);
        fs::create_dir_all(&dest_dir)?;
        let dest = dest_dir.join(file_name);

        fs::copy(source, &dest)?;

        let rel_path = format!(
            "{}/{}/{}",
            ASSETS_DIR,
            TEXTURES_DIR,
            file_name.to_string_lossy()
        );

        let name = source
            .file_stem()
            .map(|s| s.to_string_lossy().to_string())
            .unwrap_or_else(|| "texture".to_string());

        let entry = AssetEntry {
            name: name.clone(),
            path: rel_path,
        };

        if !self.project.assets.textures.iter().any(|e| e.name == name) {
            self.project.assets.textures.push(entry.clone());
        }

        self.dirty = true;
        tracing::info!("纹理已导入: {}", source.display());
        Ok(entry)
    }

    /// 获取模型在项目目录内的绝对路径
    pub fn resolve_model_path(&self, entry: &AssetEntry) -> PathBuf {
        self.project_dir.join(&entry.path)
    }

    pub fn name(&self) -> &str {
        &self.project.meta.name
    }

    /// 复制 OBJ 的伴随文件（.mtl 和 .mtl 引用的纹理）
    fn copy_obj_companions(&self, obj_source: &Path, dest_dir: &Path) {
        let Some(parent) = obj_source.parent() else {
            return;
        };

        // 复制同名 .mtl
        if let Some(stem) = obj_source.file_stem() {
            let mtl_name = format!("{}.mtl", stem.to_string_lossy());
            let mtl_source = parent.join(&mtl_name);
            if mtl_source.exists() {
                let _ = fs::copy(&mtl_source, dest_dir.join(&mtl_name));

                // 解析 .mtl 中引用的纹理文件并复制
                if let Ok(content) = fs::read_to_string(&mtl_source) {
                    for line in content.lines() {
                        let line = line.trim();
                        // map_Kd, map_Ka, map_Ks, map_Ns, map_bump 等
                        if line.starts_with("map_") || line.starts_with("bump") {
                            if let Some(tex_path_str) = line.split_whitespace().last() {
                                let tex_source = parent.join(tex_path_str);
                                if tex_source.exists() {
                                    if let Some(tex_name) = tex_source.file_name() {
                                        let _ = fs::copy(&tex_source, dest_dir.join(tex_name));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

#[derive(Debug)]
pub enum ProjectError {
    Io(std::io::Error),
    AlreadyExists(PathBuf),
    NotFound(PathBuf),
    InvalidPath(PathBuf),
    ParseError(String),
    SerializeError(String),
}

impl std::fmt::Display for ProjectError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::Io(e) => write!(f, "IO 错误: {}", e),
            Self::AlreadyExists(p) => write!(f, "项目已存在: {}", p.display()),
            Self::NotFound(p) => write!(f, "项目文件未找到: {}", p.display()),
            Self::InvalidPath(p) => write!(f, "无效路径: {}", p.display()),
            Self::ParseError(e) => write!(f, "解析错误: {}", e),
            Self::SerializeError(e) => write!(f, "序列化错误: {}", e),
        }
    }
}

impl From<std::io::Error> for ProjectError {
    fn from(e: std::io::Error) -> Self {
        Self::Io(e)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::atomic::{AtomicU64, Ordering};

    static TEST_COUNTER: AtomicU64 = AtomicU64::new(0);

    fn temp_project_dir(suffix: &str) -> PathBuf {
        let id = TEST_COUNTER.fetch_add(1, Ordering::SeqCst);
        let dir =
            std::env::temp_dir().join(format!("sge_test_{}_{}_{}", std::process::id(), id, suffix));
        let _ = fs::remove_dir_all(&dir);
        dir
    }

    #[test]
    fn create_and_open_project() {
        let dir = temp_project_dir("create_open");
        let state = ProjectState::create(&dir, "TestProject").unwrap();
        assert_eq!(state.name(), "TestProject");
        assert!(dir.join(PROJECT_FILE_NAME).exists());
        assert!(dir.join(ASSETS_DIR).join(MODELS_DIR).exists());
        assert!(dir.join(ASSETS_DIR).join(TEXTURES_DIR).exists());

        let reopened = ProjectState::open(&dir).unwrap();
        assert_eq!(reopened.name(), "TestProject");

        let _ = fs::remove_dir_all(&dir);
    }

    #[test]
    fn save_preserves_scene_config() {
        let dir = temp_project_dir("save_config");
        let mut state = ProjectState::create(&dir, "SaveTest").unwrap();
        state.project.scene.render.shininess = 128.0;
        state.project.scene.camera.fov = 90.0;
        state.save().unwrap();

        let reopened = ProjectState::open(&dir).unwrap();
        assert!((reopened.project.scene.render.shininess - 128.0).abs() < f32::EPSILON);
        assert!((reopened.project.scene.camera.fov - 90.0).abs() < f32::EPSILON);

        let _ = fs::remove_dir_all(&dir);
    }

    #[test]
    fn create_duplicate_project_fails() {
        let dir = temp_project_dir("dup");
        ProjectState::create(&dir, "Dup").unwrap();
        let result = ProjectState::create(&dir, "Dup2");
        assert!(result.is_err());

        let _ = fs::remove_dir_all(&dir);
    }

    #[test]
    fn open_nonexistent_project_fails() {
        let dir = temp_project_dir("noexist");
        let result = ProjectState::open(&dir);
        assert!(result.is_err());
    }

    #[test]
    fn toml_roundtrip() {
        let project = Project::new("RoundTrip");
        let serialized = toml::to_string_pretty(&project).unwrap();
        let deserialized: Project = toml::from_str(&serialized).unwrap();
        assert_eq!(deserialized.meta.name, "RoundTrip");
    }
}
