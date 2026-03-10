use thiserror::Error;

/// Errors that can occur in the renderer.
#[derive(Debug, Error)]
pub enum RendererError {
    #[error("Model loading failed: {0}")]
    ModelLoad(String),

    #[error("Texture loading failed: {0}")]
    TextureLoad(String),

    #[error("Rendering failed: {0}")]
    RenderFailed(String),

    #[error("IO error: {0}")]
    Io(#[from] std::io::Error),
}

/// Convenience result type for renderer operations.
pub type Result<T> = std::result::Result<T, RendererError>;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn error_display() {
        let e = RendererError::ModelLoad("file not found".into());
        assert_eq!(e.to_string(), "Model loading failed: file not found");
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
}
