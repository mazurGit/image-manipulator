#pragma once

#include "pixel-view.h"
#include <cstddef>
#include <cstdint>
#include <memory>

enum class Rotation { CW, CCW };

class Image {
public:
  // Lifetime
  Image() = default;
  Image(const Image &other);
  Image &operator=(const Image &other);
  Image(Image &&other) noexcept = default;
  Image &operator=(Image &&other) noexcept = default;

  // Input/output
  void load(const char *path);
  void save(const char *path) const;

  // Pixel access
  PixelView at(int x, int y);

  template <typename Func> void forEachPixel(Func func) {
    for (int y = 0; y < height_; y++) {
      for (int x = 0; x < width_; x++) {
        PixelView pixel = at(x, y);
        func(pixel);
      };
    }
  }

  // Transformations
  Image &grayscale();
  Image &flipHorizontal();
  Image &flipVertical();
  Image &rotate90(Rotation direction);

private:
  // Buffer ownership
  struct BufferDeleter {
    void operator()(std::uint8_t *buffer) const noexcept;
  };

  using Buffer = std::unique_ptr<std::uint8_t, BufferDeleter>;

  Buffer buffer_;
  std::size_t size_ = 0;

  // Image geometry
  int width_ = 0;
  int height_ = 0;
  int channels_ = 0;

  // Indexing helpers
  int pixelIndex(int x, int y) const;
  int pixelIndex(int x, int y, int width) const;
  std::uint8_t *pixelPtr(int x, int y);
  std::uint8_t *pixelPtr(std::uint8_t *buffer, int width, int x, int y);
};
