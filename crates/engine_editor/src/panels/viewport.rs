// Copyright The SimpleGameEngine Contributors


use egui::{ColorImage, TextureHandle, TextureOptions};

/// 将渲染器输出的 `&[u32]` RGBA 像素缓冲区转换为 egui 可用的 ColorImage。
///
/// 渲染器像素格式：小端序 RGBA，R 在低 8 位，A 在高 8 位。
/// egui ColorImage 需要 `[u8]` RGBA 排列。
pub fn pixels_to_color_image(pixels: &[u32], width: usize, height: usize) -> ColorImage {
    let mut rgba = Vec::with_capacity(width * height * 4);
    for &pixel in pixels.iter().take(width * height) {
        rgba.push((pixel & 0xFF) as u8);
        rgba.push(((pixel >> 8) & 0xFF) as u8);
        rgba.push(((pixel >> 16) & 0xFF) as u8);
        rgba.push(((pixel >> 24) & 0xFF) as u8);
    }
    ColorImage::from_rgba_unmultiplied([width, height], &rgba)
}

/// 将渲染帧上传为 egui 纹理，如已有纹理则更新，否则创建新的
pub fn upload_frame(
    ctx: &egui::Context,
    texture: &mut Option<TextureHandle>,
    pixels: &[u32],
    width: usize,
    height: usize,
) {
    let image = pixels_to_color_image(pixels, width, height);
    match texture {
        Some(tex) => {
            tex.set(image, TextureOptions::NEAREST);
        }
        None => {
            *texture = Some(ctx.load_texture("viewport", image, TextureOptions::NEAREST));
        }
    }
}

/// 显示视口面板，返回视口区域的 Response（用于检测鼠标交互）
pub fn show(ui: &mut egui::Ui, texture: &Option<TextureHandle>) -> Option<egui::Response> {
    texture.as_ref().map(|tex| {
        let available = ui.available_size();
        let tex_aspect = tex.size()[0] as f32 / tex.size()[1] as f32;
        let ui_aspect = available.x / available.y;

        // 按宽高比适配视口，保持渲染比例
        let display_size = if ui_aspect > tex_aspect {
            egui::vec2(available.y * tex_aspect, available.y)
        } else {
            egui::vec2(available.x, available.x / tex_aspect)
        };

        let image =
            egui::Image::from_texture(egui::load::SizedTexture::new(tex.id(), display_size));
        ui.add(image)
    })
}
