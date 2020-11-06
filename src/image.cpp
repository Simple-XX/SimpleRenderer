
// This file is a part of Simple-XX/SimpleRenderer
// (https://github.com/Simple-XX/SimpleRenderer).
// Based on https://github.com/ssloy/tinyrenderer
// tga.cpp for Simple-XX/SimpleRenderer.

#include "iostream"
#include "cstring"
#include "image.h"

TGAColor::TGAColor(const uint8_t _R, const uint8_t _G, const uint8_t _B,
                   const uint8_t _A)
    : bgra{_B, _G, _R, _A}, bytespp(4) {
    return;
}

TGAColor::TGAColor(const uint8_t _v) : bgra{_v, 0, 0, 0}, bytespp(1) {
    return;
}

TGAColor::TGAColor(const uint8_t *_p, const uint8_t _bpp)
    : bgra{0, 0, 0, 0}, bytespp(_bpp) {
    for (uint8_t i = 0; i < _bpp; i++) {
        bgra[i] = _p[i];
    }
    return;
}

uint8_t &TGAColor::operator[](const int _i) {
    return bgra[_i];
}

TGAColor TGAColor::operator*(const float _intensity) const {
    TGAColor res     = *this;
    float    clamped = std::max((float)0., std::min(_intensity, (float)1.));
    for (size_t i = 0; i < 4; i++) {
        res.bgra[i] = bgra[i] * clamped;
    }
    return res;
}

TGAImage::TGAImage() : data(), width(0), height(0), bytespp(0) {
    return;
}

TGAImage::TGAImage(const size_t _w, const size_t _h, const int _bpp)
    : data(_w * _h * _bpp, 0), width(_w), height(_h), bytespp(_bpp) {
    return;
}

bool TGAImage::read_tga_file(const std::string _filename) {
    std::ifstream in;
    in.open(_filename, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "can't open file " << _filename << "\n";
        in.close();
        return false;
    }
    TGA_Header header;
    in.read(reinterpret_cast<char *>(&header), sizeof(header));
    if (!in.good()) {
        in.close();
        std::cerr << "an error occured while reading the header\n";
        return false;
    }
    width   = header.width;
    height  = header.height;
    bytespp = header.bitsperpixel >> 3;
    if (width <= 0 || height <= 0 ||
        (bytespp != GRAYSCALE && bytespp != RGB && bytespp != RGBA)) {
        in.close();
        std::cerr << "bad bpp (or width/height) value\n";
        return false;
    }
    int nbytes = bytespp * width * height;
    data       = std::vector<std::uint8_t>(nbytes, 0);
    if (header.datatypecode == 3 || header.datatypecode == 2) {
        in.read(reinterpret_cast<char *>(data.data()), nbytes);
        if (!in.good()) {
            in.close();
            std::cerr << "an error occured while reading the data\n";
            return false;
        }
    }
    else if (header.datatypecode == 10 || header.datatypecode == 11) {
        if (!load_rle_data(in)) {
            in.close();
            std::cerr << "an error occured while reading the data\n";
            return false;
        }
    }
    else {
        in.close();
        std::cerr << "unknown file format " << (int)header.datatypecode << "\n";
        return false;
    }
    if (!(header.imagedescriptor & 0x20)) {
        flip_vertically();
    }
    if (header.imagedescriptor & 0x10) {
        flip_horizontally();
    }
    std::cerr << width << "x" << height << "/" << bytespp * 8 << "\n";
    in.close();
    return true;
}

bool TGAImage::load_rle_data(std::ifstream &_in) {
    int      pixelcount   = width * height;
    int      currentpixel = 0;
    int      currentbyte  = 0;
    TGAColor colorbuffer;
    do {
        std::uint8_t chunkheader = 0;
        chunkheader              = _in.get();
        if (!_in.good()) {
            std::cerr << "an error occured while reading the data\n";
            return false;
        }
        if (chunkheader < 128) {
            chunkheader++;
            for (int i = 0; i < chunkheader; i++) {
                _in.read(reinterpret_cast<char *>(colorbuffer.bgra), bytespp);
                if (!_in.good()) {
                    std::cerr << "an error occured while reading the header\n";
                    return false;
                }
                for (int t = 0; t < bytespp; t++)
                    data[currentbyte++] = colorbuffer.bgra[t];
                currentpixel++;
                if (currentpixel > pixelcount) {
                    std::cerr << "Too many pixels read\n";
                    return false;
                }
            }
        }
        else {
            chunkheader -= 127;
            _in.read(reinterpret_cast<char *>(colorbuffer.bgra), bytespp);
            if (!_in.good()) {
                std::cerr << "an error occured while reading the header\n";
                return false;
            }
            for (int i = 0; i < chunkheader; i++) {
                for (int t = 0; t < bytespp; t++) {
                    data[currentbyte++] = colorbuffer.bgra[t];
                }
                currentpixel++;
                if (currentpixel > pixelcount) {
                    std::cerr << "Too many pixels read\n";
                    return false;
                }
            }
        }
    } while (currentpixel < pixelcount);
    return true;
}

bool TGAImage::write_tga_file(const std::string _filename, const bool _vflip,
                              const bool _rle) const {
    std::uint8_t  developer_area_ref[4] = {0, 0, 0, 0};
    std::uint8_t  extension_area_ref[4] = {0, 0, 0, 0};
    std::uint8_t  footer[18] = {'T', 'R', 'U', 'E', 'V', 'I', 'S', 'I', 'O',
                               'N', '-', 'X', 'F', 'I', 'L', 'E', '.', '\0'};
    std::ofstream out;
    out.open(_filename, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "can't open file " << _filename << "\n";
        out.close();
        return false;
    }
    TGA_Header header;
    header.bitsperpixel = bytespp << 3;
    header.width        = width;
    header.height       = height;
    header.datatypecode =
        (bytespp == GRAYSCALE ? (_rle ? 11 : 3) : (_rle ? 10 : 2));
    header.imagedescriptor =
        _vflip ? 0x00 : 0x20; // top-left or bottom-left origin
    out.write(reinterpret_cast<const char *>(&header), sizeof(header));
    if (!out.good()) {
        out.close();
        std::cerr << "can't dump the tga file\n";
        return false;
    }
    if (!_rle) {
        out.write(reinterpret_cast<const char *>(data.data()),
                  width * height * bytespp);
        if (!out.good()) {
            std::cerr << "can't unload raw data\n";
            out.close();
            return false;
        }
    }
    else {
        if (!unload_rle_data(out)) {
            out.close();
            std::cerr << "can't unload rle data\n";
            return false;
        }
    }
    out.write(reinterpret_cast<const char *>(developer_area_ref),
              sizeof(developer_area_ref));
    if (!out.good()) {
        std::cerr << "can't dump the tga file\n";
        out.close();
        return false;
    }
    out.write(reinterpret_cast<const char *>(extension_area_ref),
              sizeof(extension_area_ref));
    if (!out.good()) {
        std::cerr << "can't dump the tga file\n";
        out.close();
        return false;
    }
    out.write(reinterpret_cast<const char *>(footer), sizeof(footer));
    if (!out.good()) {
        std::cerr << "can't dump the tga file\n";
        out.close();
        return false;
    }
    out.close();
    return true;
}

// TODO: it is not necessary to break a raw chunk for two equal pixels (for the
// matter of the resulting size)
bool TGAImage::unload_rle_data(std::ofstream &_out) const {
    const std::uint8_t max_chunk_length = 128;
    int                npixels          = width * height;
    int                curpix           = 0;
    while (curpix < npixels) {
        int          chunkstart = curpix * bytespp;
        int          curbyte    = curpix * bytespp;
        std::uint8_t run_length = 1;
        bool         raw        = true;
        while (curpix + run_length < npixels && run_length < max_chunk_length) {
            bool succ_eq = true;
            for (int t = 0; succ_eq && t < bytespp; t++) {
                succ_eq = (data[curbyte + t] == data[curbyte + t + bytespp]);
            }
            curbyte += bytespp;
            if (1 == run_length) {
                raw = !succ_eq;
            }
            if (raw && succ_eq) {
                run_length--;
                break;
            }
            if (!raw && !succ_eq)
                break;
            run_length++;
        }
        curpix += run_length;
        _out.put(raw ? run_length - 1 : run_length + 127);
        if (!_out.good()) {
            std::cerr << "can't dump the tga file\n";
            return false;
        }
        _out.write(reinterpret_cast<const char *>(data.data() + chunkstart),
                   (raw ? run_length * bytespp : bytespp));
        if (!_out.good()) {
            std::cerr << "can't dump the tga file\n";
            return false;
        }
    }
    return true;
}

TGAColor TGAImage::get(const size_t _x, const size_t _y) const {
    if (!data.size() || _x >= width || _y >= height) {
        return {};
    }
    return TGAColor(data.data() + (_x + _y * width) * bytespp, bytespp);
}

void TGAImage::set(size_t _x, size_t _y, const TGAColor &_c) {
    if (!data.size() || _x >= width || _y >= height) {
        return;
    }
    memcpy(data.data() + (_x + _y * width) * bytespp, _c.bgra, bytespp);
}

int TGAImage::get_bytespp() {
    return bytespp;
}

size_t TGAImage::get_width() const {
    return width;
}

size_t TGAImage::get_height() const {
    return height;
}

void TGAImage::flip_horizontally() {
    if (!data.size()) {
        return;
    }
    size_t half = width >> 1;
    for (size_t i = 0; i < half; i++) {
        for (size_t j = 0; j < height; j++) {
            TGAColor c1 = get(i, j);
            TGAColor c2 = get(width - 1 - i, j);
            set(i, j, c2);
            set(width - 1 - i, j, c1);
        }
    }
    return;
}

void TGAImage::flip_vertically() {
    if (!data.size()) {
        return;
    }
    int                       bytes_per_line = width * bytespp;
    std::vector<std::uint8_t> line(bytes_per_line, 0);
    int                       half = height >> 1;
    for (int j = 0; j < half; j++) {
        int l1 = j * bytes_per_line;
        int l2 = (height - 1 - j) * bytes_per_line;
        std::copy(data.begin() + l1, data.begin() + l1 + bytes_per_line,
                  line.begin());
        std::copy(data.begin() + l2, data.begin() + l2 + bytes_per_line,
                  data.begin() + l1);
        std::copy(line.begin(), line.end(), data.begin() + l2);
    }
    return;
}

std::uint8_t *TGAImage::buffer() {
    return data.data();
}

void TGAImage::clear() {
    data = std::vector<std::uint8_t>(width * height * bytespp, 0);
    return;
}

void TGAImage::scale(int _w, int _h) {
    if (_w <= 0 || _h <= 0 || !data.size()) {
        return;
    }
    std::vector<std::uint8_t> tdata(_w * _h * bytespp, 0);
    int                       nscanline  = 0;
    int                       oscanline  = 0;
    int                       erry       = 0;
    int                       nlinebytes = _w * bytespp;
    int                       olinebytes = width * bytespp;
    for (size_t j = 0; j < height; j++) {
        size_t errx = width - _w;
        int    nx   = -bytespp;
        int    ox   = -bytespp;
        for (size_t i = 0; i < width; i++) {
            ox += bytespp;
            errx += _w;
            while (errx >= width) {
                errx -= width;
                nx += bytespp;
                memcpy(tdata.data() + nscanline + nx,
                       data.data() + oscanline + ox, bytespp);
            }
        }
        erry += _h;
        oscanline += olinebytes;
        while (erry >= (int)height) {
            if (erry >= (int)height << 1) {
                // it means we jump over a scanline
                memcpy(tdata.data() + nscanline + nlinebytes,
                       tdata.data() + nscanline, nlinebytes);
            }
            erry -= height;
            nscanline += nlinebytes;
        }
    }
    data   = tdata;
    width  = _w;
    height = _h;
    return;
}
