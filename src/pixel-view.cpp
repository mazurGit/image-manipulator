#include "pixel-view.h"
#include "color.h"
#include "pixel-math.h"
#include <cstdint>

PixelView::PixelView(uint8_t *data, int channels)
    : data_{data}, channels_{channels} {}

uint8_t &PixelView::getColor(Channel channel) {
  return getColorImpl(*this, channel);
}

const uint8_t &PixelView::getColor(Channel channel) const {
  return getColorImpl(*this, channel);
}

Color PixelView::toColor() const noexcept {
  return Color{data_[0], data_[1], data_[2], data_[3]};
}

std::ostream &operator<<(std::ostream &os, const PixelView &pixel) {
  os << "Pixel(";
  for (int channel = 0; channel < pixel.channels_; ++channel) {
    if (channel != 0) {
      os << ", ";
    }
    os << static_cast<int>(pixel.data_[channel]);
  }
  return os << ')';
}

uint8_t PixelView::luma() const {
  float result = 0.0f;
  forEachChannel([&](const uint8_t &color, int i) {
    result += color * pixel_math::kLuminanceWeights[i];
  });
  return static_cast<uint8_t>(result);
}

PixelView &PixelView::invert() noexcept {
  forEachChannel([](std::uint8_t &color, int) { color = 255 - color; });
  return *this;
}

PixelView &PixelView::adjustBrightness(int difference) noexcept {
  forEachChannel([difference](std::uint8_t &color, int) {
    color = pixel_math::clampToByte(static_cast<std::int64_t>(color) +
                                    difference);
  });
  return *this;
}

PixelView &PixelView::operator=(uint8_t uniColor) {
  forEachChannel([&](uint8_t &color, int) { color = uniColor; });
  return *this;
}

PixelView &PixelView::operator=(const PixelView &other) noexcept {
  if (data_ != other.data_) {
    std::copy_n(other.data_, std::min(channels_, other.channels_), data_);
  }
  return *this;
}

void swapPixel(PixelView a, PixelView b) noexcept {
  int count = std::min(a.channels_, b.channels_);
  for (int i = 0; i < count; ++i) {
    std::swap(a.data_[i], b.data_[i]);
  }
}
