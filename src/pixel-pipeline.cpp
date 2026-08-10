#include "pixel-pipeline.h"
#include <cmath>
#include <stdexcept>

void PixelPipeline::Grayscale::apply(PixelView &pixel) const noexcept {
  pixel = pixel.luma();
}

void PixelPipeline::Invert::apply(PixelView &pixel) const noexcept {
  pixel.invert();
}

void PixelPipeline::Brightness::apply(PixelView &pixel) const noexcept {
  pixel.adjustBrightness(difference);
}

PixelPipeline::Contrast::Contrast(float factor) : factor{factor} {
  if (!std::isfinite(factor)) {
    throw std::invalid_argument("contrast factor must be finite");
  }
}

void PixelPipeline::Contrast::apply(PixelView &pixel) const noexcept {
  pixel.adjustContrast(factor);
}

void PixelPipeline::Threshold::apply(PixelView &pixel) const noexcept {
  pixel.applyThreshold(value);
}

Image &PixelPipeline::apply(Image &image,
                            const std::vector<Operation> &operations) {
  image.forEachPixel([&operations](PixelView &pixel) {
    for (const Operation &operation : operations) {
      std::visit([&pixel](const auto &value) { value.apply(pixel); },
                 operation);
    }
  });
  return image;
}
