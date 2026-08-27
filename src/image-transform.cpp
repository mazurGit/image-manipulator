#include "image.h"
#include <algorithm>
#include <cstdint>
#include <utility>
#include <glib.h>

Image &Image::flipHorizontal() {
  const int xCenter = width_ / 2;
  for (int y = 0; y < height_; y++) {
    Row currentRow = (*this)[y];
    for (int x = 0; x < xCenter; x++) {
      swapPixel(currentRow[x], currentRow[width_ - 1 - x]);
    }
  }
  return *this;
};

Image &Image::flipVertical() {
  const int yCenter = height_ / 2;
  for (int y = 0; y < yCenter; y++) {
    Row topRow = (*this)[y];
    Row bottomRow = (*this)[height_ - 1 - y];
    for (int x = 0; x < width_; x++) {
      swapPixel(topRow[x], bottomRow[x]);
    }
  }
  return *this;
};

Image &Image::rotate90(Rotation direction) {
  int newWidth = height_;
  int newHeight = width_;

  Buffer temp{static_cast<std::uint8_t *>(g_malloc(size_))};
  bool isCW = direction == Rotation::CW;
  for (int y = 0; y < height_; y++) {
    for (int x = 0; x < width_; x++) {
      int newX = isCW ? height_ - 1 - y : y;
      int newY = isCW ? x : width_ - 1 - x;
      std::copy_n(pixelPtr(x, y), channels_,
                  pixelPtr(temp.get(), newWidth, newX, newY));
    }
  }
  replaceBuffer(std::move(temp), size_, newWidth, newHeight);
  return *this;
}
