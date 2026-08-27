#include "image.h"
#include "pixel-math.h"
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>
#include <glib.h>

Image &Image::resize(int width, int height, ResizeFilter filter) {
  if (!buffer_) {
    throw std::runtime_error("cannot resize an empty image");
  }
  const std::size_t newSize = checkedBufferSize(width, height, channels_);
  if (width == width_ && height == height_) {
    return *this;
  }

  switch (filter) {
  case ResizeFilter::NearestNeighbor:
    return resizeNearest(width, height, newSize);
  case ResizeFilter::Bilinear:
    return resizeBilinear(width, height, newSize);
  }
  throw std::invalid_argument("unsupported resize filter");
}

Image &Image::resizeNearest(int width, int height, std::size_t newSize) {
  Buffer temp{static_cast<std::uint8_t *>(g_malloc(newSize))};

  for (int y = 0; y < height; y++) {
    const int srcY =
        static_cast<int>(static_cast<std::int64_t>(y) * height_ / height);
    Row destinationRow = row(temp, width, y);
    Row sourceRow = (*this)[srcY];

    for (int x = 0; x < width; x++) {
      const int srcX =
          static_cast<int>(static_cast<std::int64_t>(x) * width_ / width);
      destinationRow[x] = sourceRow[srcX];
    }
  }

  replaceBuffer(std::move(temp), newSize, width, height);
  return *this;
};

Image &Image::resizeBilinear(int width, int height, std::size_t newSize) {
  struct XInfo {
    int left;
    int right;
    float mix;
  };

  const float scaleX = static_cast<float>(width_) / width;
  const float scaleY = static_cast<float>(height_) / height;
  std::vector<XInfo> xTable(width);

  for (int x = 0; x < width; x++) {
    const float sourceX = x * scaleX;
    const int left = static_cast<int>(sourceX);
    xTable[x] = {left, std::min(left + 1, width_ - 1), sourceX - left};
  }

  Buffer temp{static_cast<std::uint8_t *>(g_malloc(newSize))};
  const std::size_t stride = static_cast<std::size_t>(width_) * channels_;
  std::uint8_t *destination = temp.get();

  for (int y = 0; y < height; y++) {
    const float sourceY = y * scaleY;
    const int top = static_cast<int>(sourceY);
    const int bottom = std::min(top + 1, height_ - 1);
    const float yMix = sourceY - top;
    const std::uint8_t *topRow = buffer_.get() + top * stride;
    const std::uint8_t *bottomRow = buffer_.get() + bottom * stride;

    for (int x = 0; x < width; x++) {
      const auto [left, right, xMix] = xTable[x];
      const std::uint8_t *topLeft = topRow + left * channels_;
      const std::uint8_t *topRight = topRow + right * channels_;
      const std::uint8_t *bottomLeft = bottomRow + left * channels_;
      const std::uint8_t *bottomRight = bottomRow + right * channels_;

      for (int channel = 0; channel < channels_; channel++) {
        const float upper =
            pixel_math::lerp(topLeft[channel], topRight[channel], xMix);
        const float lower =
            pixel_math::lerp(bottomLeft[channel], bottomRight[channel], xMix);
        *destination++ = static_cast<std::uint8_t>(
            pixel_math::lerp(upper, lower, yMix) + 0.5f);
      }
    }
  }

  replaceBuffer(std::move(temp), newSize, width, height);
  return *this;
};
