#pragma once

#include "pixel-view.h"
#include <cstddef>
#include <vector>

class Image {
private:
  size_t size = 0;
  int width_ = 0;
  int height_ = 0;
  int channels_ = 0;
  std::vector<uint8_t> buffer_;
  int pixelIndex(int x, int y) const;

public:
  void load(const char *path);
  void save(const char *path) const;
  PixelView at(int x, int y);
  Image &grayscale();

  template <typename Func> void forEachPixel(Func func) {
    for (int y = 0; y < height_; y++) {
      for (int x = 0; x < width_; x++) {
        PixelView pixel = at(x, y);
        func(pixel);
      };
    }
  }
};
