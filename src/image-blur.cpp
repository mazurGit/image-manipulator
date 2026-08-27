#include "image.h"
#include "color-accumulator.h"
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <glib.h>

Image &Image::blur(int radius) {
  if (!buffer_) {
    throw std::runtime_error("cannot blur an empty image");
  }
  if (radius < 0) {
    throw std::invalid_argument("blur radius cannot be negative");
  }
  if (radius == 0) {
    return *this;
  }

  Buffer temp{static_cast<std::uint8_t *>(g_malloc(size_))};
  const std::size_t stride = static_cast<std::size_t>(width_) * channels_;

  for (int y = 0; y < height_; ++y) {
    ColorAccumulator accumulator;
    const std::uint8_t *sourceRow = buffer_.get() + y * stride;
    std::uint8_t *destinationRow = temp.get() + y * stride;
    const int initialRight = std::min(radius, width_ - 1);

    for (int sampleX = 0; sampleX <= initialRight; ++sampleX) {
      accumulator.add(sourceRow + sampleX * channels_, channels_);
    }

    for (int x = 0; x < width_; ++x) {
      accumulator.writeAverage(destinationRow + x * channels_, channels_);

      const int removedX = x - radius;
      const int addedX = x + radius + 1;

      if (removedX >= 0) {
        accumulator.remove(sourceRow + removedX * channels_, channels_);
      }

      if (addedX < width_) {
        accumulator.add(sourceRow + addedX * channels_, channels_);
      }
    }
  }

  std::vector<ColorAccumulator> columns(width_);
  const int initialBottom = std::min(radius, height_ - 1);
  for (int sampleY = 0; sampleY <= initialBottom; ++sampleY) {
    const std::uint8_t *sourceRow = temp.get() + sampleY * stride;
    for (int x = 0; x < width_; ++x) {
      columns[x].add(sourceRow + x * channels_, channels_);
    }
  }

  for (int y = 0; y < height_; ++y) {
    const int removedY = y - radius;
    const int addedY = y + radius + 1;
    std::uint8_t *destinationRow = buffer_.get() + y * stride;
    const std::uint8_t *removedRow =
        removedY >= 0 ? temp.get() + removedY * stride : nullptr;
    const std::uint8_t *addedRow =
        addedY < height_ ? temp.get() + addedY * stride : nullptr;

    for (int x = 0; x < width_; ++x) {
      columns[x].writeAverage(destinationRow + x * channels_, channels_);
      if (removedY >= 0) {
        columns[x].remove(removedRow + x * channels_, channels_);
      }
      if (addedY < height_) {
        columns[x].add(addedRow + x * channels_, channels_);
      }
    }
  }

  return *this;
};
