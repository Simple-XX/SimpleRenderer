// Copyright The SimpleRenderer Contributors

use super::Shader;

// ── 常量 ──────────────────────────────────────────────────────────────────

pub(crate) const SPECULAR_LUT_RESOLUTION: usize = 256;

// ── 高光查找表 ────────────────────────────────────────────────────────────

/// 预计算的 `cos_theta^shininess` 查找表。
#[derive(Clone)]
pub(crate) struct SpecularLut {
    pub(crate) values: [f32; SPECULAR_LUT_RESOLUTION],
}

impl Shader {
    // ── 高光查找表（私有）─────────────────────────────────────────────

    /// 构建 `cos_theta^shininess` 的查找表。
    pub(super) fn build_specular_lut(shininess: f32) -> SpecularLut {
        let mut values = [0.0_f32; SPECULAR_LUT_RESOLUTION];
        if shininess <= 0.0 {
            values.fill(1.0);
            return SpecularLut { values };
        }
        for (i, value) in values.iter_mut().enumerate().take(SPECULAR_LUT_RESOLUTION) {
            let cos_theta = i as f32 / (SPECULAR_LUT_RESOLUTION - 1) as f32;
            *value = if cos_theta <= 0.0 {
                0.0
            } else {
                cos_theta.powf(shininess)
            };
        }
        SpecularLut { values }
    }

    /// 获取或创建给定 shininess 的缓存高光查找表。
    ///
    /// 返回 LUT 值的克隆（`RwLock` 不允许返回引用）。
    pub(super) fn get_specular_lut(&self, shininess: f32) -> [f32; SPECULAR_LUT_RESOLUTION] {
        let key = shininess.to_bits();

        // 先尝试读锁
        {
            let cache = self
                .specular_lut_cache
                .read()
                .unwrap_or_else(|e| e.into_inner());
            if let Some(lut) = cache.get(&key) {
                return lut.values;
            }
        }

        let lut = Self::build_specular_lut(shininess);
        let values = lut.values;
        let mut cache = self
            .specular_lut_cache
            .write()
            .unwrap_or_else(|e| e.into_inner());
        cache.entry(key).or_insert(lut);
        values
    }

    /// 使用缓存的查找表通过线性插值计算高光贡献。
    pub(super) fn evaluate_specular(&self, cos_theta: f32, shininess: f32) -> f32 {
        let cos_theta = cos_theta.clamp(0.0, 1.0);
        if shininess <= 0.0 {
            return 1.0;
        }
        if cos_theta <= 0.0 {
            return 0.0;
        }

        let lut = self.get_specular_lut(shininess);
        let scaled = cos_theta * (SPECULAR_LUT_RESOLUTION - 1) as f32;
        let index = scaled as usize;
        let frac = scaled - index as f32;

        let v0 = lut[index];
        let v1 = lut[(index + 1).min(SPECULAR_LUT_RESOLUTION - 1)];
        v0 + (v1 - v0) * frac
    }
}
