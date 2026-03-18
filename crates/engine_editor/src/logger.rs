//! 编辑器日志系统 — 自定义 tracing Layer，同时输出到 stderr 和内存缓冲区。

use std::collections::VecDeque;
use std::sync::{Arc, Mutex};
use std::time::Instant;

use tracing::Level;
use tracing_subscriber::prelude::*;

/// 单条日志记录
pub struct LogEntry {
    /// 相对于程序启动的时间戳（秒）
    pub timestamp: f64,
    /// 日志级别
    pub level: Level,
    /// 日志来源模块
    pub target: String,
    /// 日志消息内容
    pub message: String,
}

/// 日志缓冲区的共享句柄类型
pub type LogBuffer = Arc<Mutex<VecDeque<LogEntry>>>;

/// 编辑器自定义 tracing Layer
struct EditorLayer {
    buffer: LogBuffer,
    max_entries: usize,
    start_time: Instant,
}

/// 从 tracing Event 中提取 message 字段的访问者
#[derive(Default)]
struct MessageVisitor {
    message: String,
}

impl tracing::field::Visit for MessageVisitor {
    fn record_debug(&mut self, field: &tracing::field::Field, value: &dyn std::fmt::Debug) {
        if field.name() == "message" {
            self.message = format!("{:?}", value);
        }
    }

    fn record_str(&mut self, field: &tracing::field::Field, value: &str) {
        if field.name() == "message" {
            self.message = value.to_string();
        }
    }
}

impl<S: tracing::Subscriber> tracing_subscriber::Layer<S> for EditorLayer {
    fn on_event(
        &self,
        event: &tracing::Event<'_>,
        _ctx: tracing_subscriber::layer::Context<'_, S>,
    ) {
        let metadata = event.metadata();
        let level = *metadata.level();
        let target = metadata.target().to_string();

        let mut visitor = MessageVisitor::default();
        event.record(&mut visitor);

        let elapsed = self.start_time.elapsed().as_secs_f64();

        if let Ok(mut buf) = self.buffer.lock() {
            while buf.len() >= self.max_entries {
                buf.pop_front();
            }
            buf.push_back(LogEntry {
                timestamp: elapsed,
                level,
                target,
                message: visitor.message,
            });
        }
    }
}

/// 初始化编辑器日志系统，返回日志缓冲区句柄。
///
/// 调用后所有 `tracing::info!()` 等宏产生的日志会同时输出到
/// stderr 和返回的 `LogBuffer` 中。
pub fn init_logger(max_entries: usize) -> LogBuffer {
    let buffer: LogBuffer = Arc::new(Mutex::new(VecDeque::with_capacity(max_entries)));

    let editor_layer = EditorLayer {
        buffer: Arc::clone(&buffer),
        max_entries,
        start_time: Instant::now(),
    };

    // fmt layer 负责输出到 stderr（替代原来 EditorLogger 中的 eprintln）
    let fmt_layer = tracing_subscriber::fmt::layer()
        .with_target(true)
        .with_writer(std::io::stderr);

    tracing_subscriber::registry()
        .with(editor_layer)
        .with(fmt_layer.with_filter(tracing_subscriber::filter::LevelFilter::DEBUG))
        .init();

    buffer
}

/// 清空日志缓冲区
pub fn clear_log_buffer(buffer: &LogBuffer) {
    if let Ok(mut buf) = buffer.lock() {
        buf.clear();
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn log_entry_fields() {
        let entry = LogEntry {
            timestamp: 1.234,
            level: Level::INFO,
            target: "test".to_string(),
            message: "hello".to_string(),
        };
        assert_eq!(entry.level, Level::INFO);
        assert_eq!(entry.message, "hello");
    }
}
