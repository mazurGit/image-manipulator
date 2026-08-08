#include "pixel-view.h"
#include <cstdint>

PixelView::PixelView(uint8_t *data, int channels)
    : data_{data}, channels_{channels} 
    {}

uint8_t &PixelView::getColor(Channel channel) {
  return getColorImpl(*this, channel);
}

const uint8_t &PixelView::getColor(Channel channel) const {
  return getColorImpl(*this, channel);
}

std::ostream& operator<<(
    std::ostream& os,
    const PixelView& pixel
) {
    os << "Pixel(";
    for (int channel = 0; channel < pixel.channels_; ++channel) {
        if (channel != 0) {
            os << ", ";
        }
        os << static_cast<int>(pixel.data_[channel]);
    }
    return os << ')';
}