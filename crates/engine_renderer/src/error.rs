// Copyright (c) Simple-XX/SimpleRenderer
// SPDX-License-Identifier: MIT

use thiserror::Error;

/// 渲染器中可能出现的错误。
#[non_exhaustive]
#[derive(Debug, Error)]
pub enum RendererError {
    #[error("Model loading failed: {path}")]
    ModelLoad {
        path: String,
        #[source]
        source: Box<dyn std::error::Error + Send + Sync>,
    },

    #[error("Texture loading failed: {path}")]
    TextureLoad {
        path: String,
        #[source]
        source: Box<dyn std::error::Error + Send + Sync>,
    },

    #[error("Rendering failed: {0}")]
    RenderFailed(String),

    #[error("IO error: {0}")]
    Io(#[from] std::io::Error),
}

/// 渲染器操作的便捷 Result 类型别名。
pub type Result<T> = std::result::Result<T, RendererError>;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn error_display() {
        let e = RendererError::ModelLoad {
            path: "test.obj".into(),
            source: "file not found".into(),
        };
        assert!(e.to_string().contains("test.obj"));
    }

    #[test]
    fn error_from_io() {
        let io_err = std::io::Error::new(std::io::ErrorKind::NotFound, "gone");
        let e: RendererError = io_err.into();
        assert!(matches!(e, RendererError::Io(_)));
        assert!(e.to_string().contains("gone"));
    }

    #[test]
    fn result_type_alias_works() {
        let ok: Result<i32> = Ok(42);
        assert_eq!(ok.unwrap(), 42);

        let err: Result<i32> = Err(RendererError::RenderFailed("oops".into()));
        assert!(err.is_err());
    }

    #[test]
    fn error_source_chain() {
        use std::error::Error;
        let e = RendererError::ModelLoad {
            path: "model.obj".into(),
            source: Box::new(std::io::Error::new(std::io::ErrorKind::NotFound, "missing")),
        };
        assert!(e.source().is_some());
    }
}
