
// This file is a part of SimpleXX/SimpleRenderer
// (https://github.com/SimpleXX/SimpleRenderer). Based on
// https://github.com/ssloy/tinyrenderer
//
// image.h for SimpleXX/SimpleRenderer.

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "cstdint"
#include "fstream"
#include "vector"

#pragma pack(push, 1)
struct TGA_Header {
    std::uint8_t  idlength{};
    std::uint8_t  colormaptype{};
    std::uint8_t  datatypecode{};
    std::uint16_t colormaporigin{};
    std::uint16_t colormaplength{};
    std::uint8_t  colormapdepth{};
    std::uint16_t x_origin{};
    std::uint16_t y_origin{};
    std::uint16_t width{};
    std::uint16_t height{};
    std::uint8_t  bitsperpixel{};
    std::uint8_t  imagedescriptor{};
};
#pragma pack(pop)

struct TGAColor {
    std::uint8_t bgra[4] = {0, 0, 0, 0};
    std::uint8_t bytespp = {0};
    TGAColor()           = default;
    TGAColor(const std::uint8_t _R, const std::uint8_t _G,
             const std::uint8_t _B, const std::uint8_t _A = 255);
    TGAColor(const std::uint8_t _v);
    TGAColor(const std::uint8_t *_p, const std::uint8_t _bpp);
    std::uint8_t &operator[](const int _i);
    TGAColor      operator*(const float _intensity) const;
};

class TGAImage {
protected:
    std::vector<std::uint8_t> data;
    int                       width;
    int                       height;
    int                       bytespp;
    bool                      load_rle_data(std::ifstream &_in);
    bool                      unload_rle_data(std::ofstream &_out) const;

public:
    enum Format { GRAYSCALE = 1, RGB = 3, RGBA = 4 };
    TGAImage();
    TGAImage(const int _w, const int _h, const int _bpp);
    bool read_tga_file(const std::string _filename);
    bool write_tga_file(const std::string _filename, const bool _vflip = true,
                        const bool _rle = true) const;
    void flip_horizontally();
    void flip_vertically();
    void scale(const int _w, const int _h);
    TGAColor      get(const int _x, const int _y) const;
    void          set(const int _x, const int _y, const TGAColor &_c);
    int           get_width() const;
    int           get_height() const;
    int           get_bytespp();
    std::uint8_t *buffer();
    void          clear();
};

#endif /* __IMAGE_H__ */
