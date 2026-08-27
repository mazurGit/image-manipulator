#include "image.h"
#include <cmath>
#include <stdexcept>

Image &Image::grayscale() {
  forEachPixel([](PixelView &pixel) -> void { pixel = pixel.luma(); });
  return *this;
};

Image &Image::invert() {
  forEachPixel([](PixelView &pixel) { pixel.invert(); });
  return *this;
}

Image &Image::brightness(int difference) {
  forEachPixel(
      [difference](PixelView &pixel) { pixel.adjustBrightness(difference); });
  return *this;
}

Image &Image::contrast(float factor) {
  if (!std::isfinite(factor)) {
    throw std::invalid_argument("contrast factor must be finite");
  }
  forEachPixel([factor](PixelView &pixel) { pixel.adjustContrast(factor); });
  return *this;
}

Image &Image::threshold(std::uint8_t value) {
  forEachPixel([value](PixelView &pixel) { pixel.applyThreshold(value); });
  return *this;
}
