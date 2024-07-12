#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <vector>
#include <cstdint>


class Framebuffer {
public:
    Framebuffer(size_t width, size_t height)
        : width_(width), height_(height), buffer_(width * height) {}
    
    size_t width() const { return width_; }
    size_t height() const { return height_; }
    uint32_t *data() { return buffer_.data(); }
    
private:
    size_t width_, height_;
    std::vector<uint32_t> buffer_;
};

#endif // FRAMEBUFFER_H