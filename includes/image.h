#pragma  once

#include <cstddef>
#include <vector>
#include "pixel-view.h"

class Image {
    private:
    size_t size = 0;
    int width_ = 0;
    int height_ = 0;
    int channels_ = 0;
    std::vector<uint8_t> buffer_;
    int pixelIndex(int x, int y) const;

    public:
    void load( const char * path);
    PixelView at (int x, int y);
};