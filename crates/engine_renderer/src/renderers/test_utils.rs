// Copyright The SimpleGameEngine Contributors


#[cfg(test)]
mod tests {
    use crate::color::Color;
    use crate::light::Light;
    use crate::math::{Mat4, Vec3};
    use crate::model::Model;
    use crate::shader::Shader;

    pub fn test_shader() -> Shader {
        let mut shader = Shader::new();
        shader.set_uniform("modelMatrix", Mat4::IDENTITY);
        shader.set_uniform("viewMatrix", Mat4::IDENTITY);
        shader.set_uniform("projectionMatrix", Mat4::IDENTITY);
        shader.set_lights(&[Light {
            name: "test".to_string(),
            position: Vec3::ZERO,
            direction: Vec3::new(0.0, 0.0, 1.0),
            color: Color::WHITE,
        }]);
        shader.set_uniform("cameraPos", Vec3::new(0.0, 0.0, 5.0));
        shader
    }

    pub fn create_test_model(
        positions: &[[f32; 3]],
        normal: [f32; 3],
        face_indices: &[[usize; 3]],
    ) -> Model {
        use std::io::Write;
        use std::sync::atomic::{AtomicUsize, Ordering};

        static COUNTER: AtomicUsize = AtomicUsize::new(0);
        let id = COUNTER.fetch_add(1, Ordering::SeqCst);
        let path = std::env::temp_dir().join(format!("engine_renderer_test_{}.obj", id));

        let mut file = std::fs::File::create(&path).unwrap();
        for pos in positions {
            writeln!(file, "v {} {} {}", pos[0], pos[1], pos[2]).unwrap();
        }
        for _ in positions {
            writeln!(file, "vn {} {} {}", normal[0], normal[1], normal[2]).unwrap();
        }
        for face in face_indices {
            writeln!(
                file,
                "f {}//{} {}//{} {}//{}",
                face[0] + 1,
                face[0] + 1,
                face[1] + 1,
                face[1] + 1,
                face[2] + 1,
                face[2] + 1,
            )
            .unwrap();
        }
        drop(file);

        let model = Model::load(path.to_str().unwrap()).unwrap();
        let _ = std::fs::remove_file(&path);
        model
    }
}

pub use tests::{create_test_model, test_shader};
