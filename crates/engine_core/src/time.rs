// Copyright The SimpleGameEngine Contributors


/// 帧计时信息。
pub struct Time {
    /// 上一帧到当前帧的时间间隔（秒）
    pub delta: f32,
    /// 累计经过的总时间（秒）
    pub total: f32,
    /// 总帧数
    pub frame_count: u64,
}

impl Default for Time {
    fn default() -> Self {
        Self::new()
    }
}

impl Time {
    /// 创建初始计时状态（全部归零）。
    pub fn new() -> Self {
        Self {
            delta: 0.0,
            total: 0.0,
            frame_count: 0,
        }
    }

    /// 推进一帧，累加时间和帧数。
    pub fn update(&mut self, delta_seconds: f32) {
        self.delta = delta_seconds;
        self.total += delta_seconds;
        self.frame_count += 1;
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn 初始状态全部为零() {
        let time = Time::new();
        assert!((time.delta).abs() < f32::EPSILON);
        assert!((time.total).abs() < f32::EPSILON);
        assert_eq!(time.frame_count, 0);
    }

    #[test]
    fn update正确累加时间和帧数() {
        let mut time = Time::new();

        time.update(0.016);
        assert!((time.delta - 0.016).abs() < f32::EPSILON);
        assert!((time.total - 0.016).abs() < 1e-6);
        assert_eq!(time.frame_count, 1);

        time.update(0.033);
        assert!((time.delta - 0.033).abs() < f32::EPSILON);
        assert!((time.total - 0.049).abs() < 1e-5);
        assert_eq!(time.frame_count, 2);
    }

    #[test]
    fn 多次更新后累计正确() {
        let mut time = Time::new();
        for _ in 0..100 {
            time.update(0.016);
        }
        assert_eq!(time.frame_count, 100);
        assert!((time.total - 1.6).abs() < 0.01);
    }

    #[test]
    fn default与new一致() {
        let time = Time::default();
        assert!((time.delta).abs() < f32::EPSILON);
        assert_eq!(time.frame_count, 0);
    }
}
