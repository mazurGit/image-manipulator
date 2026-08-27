#include "image.h"
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <glib.h>

Image &Image::crop(int y, int x, int size, CropShape shape) {
  if (!buffer_) {
    throw std::runtime_error("cannot crop an empty image");
  }
  if (size <= 0) {
    throw std::invalid_argument("crop size must be positive");
  }
  if (y < 0 || x < 0 || y > height_ - size || x > width_ - size) {
    throw std::out_of_range("crop area is out of image bounds");
  }

  switch (shape) {
  case CropShape::Square:
    return cropSquare(y, x, size);
  case CropShape::Circle:
    return cropCircle(y, x, size);
  }
  throw std::invalid_argument("unsupported crop shape");
}

Image &Image::cropSquare(int y, int x, int size) {
  const std::size_t newSize = checkedBufferSize(size, size, channels_);
  const std::size_t rowSize = static_cast<std::size_t>(size) * channels_;
  Buffer temp{static_cast<std::uint8_t *>(g_malloc(newSize))};

  for (int cropY = 0; cropY < size; cropY++) {
    std::memcpy(temp.get() + static_cast<std::size_t>(cropY) * rowSize,
                pixelPtr(x, y + cropY), rowSize);
  }

  replaceBuffer(std::move(temp), newSize, size, size);
  return *this;
}

Image &Image::cropCircle(int y, int x, int size) {
  const std::size_t newSize = checkedBufferSize(size, size, channels_);

  Buffer temp{static_cast<std::uint8_t *>(g_malloc0(newSize))};
  const int radius = size / 2;
  for (int row = 0; row < size; ++row) {
    const int dy = std::abs(row - radius);
    if (dy > radius) {
      continue;
    }
    const int halfWidth =
        static_cast<int>(std::sqrt(radius * radius - dy * dy));
    const int left = radius - halfWidth;
    const int right = radius + halfWidth;
    const int pixelCount = right - left + 1;

    const std::size_t sourceIndex =
        (static_cast<std::size_t>(y + row) * width_ + (x + left)) * channels_;

    const std::size_t destinationIndex =
        (static_cast<std::size_t>(row) * size + left) * channels_;

    const std::size_t byteCount =
        static_cast<std::size_t>(pixelCount) * channels_;

    std::memcpy(temp.get() + destinationIndex, buffer_.get() + sourceIndex,
                byteCount);
  }

  replaceBuffer(std::move(temp), newSize, size, size);
  return *this;
}
