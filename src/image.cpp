#include "image.h"
#include <vips/vips8>

void Image::load( const char * path){
    auto image = vips::VImage::new_from_file(path)
        .colourspace(VIPS_INTERPRETATION_sRGB)
        .cast(VIPS_FORMAT_UCHAR);
    if (image.bands() == 3) {
        image = image.addalpha();
    }
    void* data = image.write_to_memory(&size);
    width_ = image.width();
    height_ = image.height();
    channels_ = image.bands();
    const auto* bytes =
        static_cast<const std::uint8_t*>(data);

    buffer_.assign(bytes, bytes + size);
    g_free(data);
};

int Image::pixelIndex(int x, int y) const {
     return (y * width_ + x) * channels_;
}

PixelView Image::at (int x, int y ){
    int index = pixelIndex(x,y);

    PixelView pixel {buffer_.data()+index, channels_};
    return  pixel;
}
