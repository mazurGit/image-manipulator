#include "image.h"
#include "pixel-view.h"
#include <cstring>
#include <stdexcept>
#include <utility>
#include <vips/vips8>

void Image::BufferDeleter::operator()(std::uint8_t *buffer) const noexcept {
  g_free(buffer);
}

Image::Image(const Image &other) { *this = other; }

Image &Image::operator=(const Image &other) {
  if (this == &other) {
    return *this;
  }

  Buffer buffer;
  if (other.buffer_) {
    buffer.reset(static_cast<std::uint8_t *>(g_malloc(other.size_)));
    std::memcpy(buffer.get(), other.buffer_.get(), other.size_);
  }

  buffer_ = std::move(buffer);
  size_ = other.size_;
  width_ = other.width_;
  height_ = other.height_;
  channels_ = other.channels_;
  return *this;
}

void Image::load(const char *path) {
  auto image = vips::VImage::new_from_file(path)
                   .colourspace(VIPS_INTERPRETATION_sRGB)
                   .cast(VIPS_FORMAT_UCHAR);
  if (image.bands() == 3) {
    image = image.addalpha();
  }

  std::size_t size = 0;
  Buffer buffer{static_cast<std::uint8_t *>(image.write_to_memory(&size))};

  // Commit only after decoding succeeds, preserving the previous image on
  // error.
  buffer_ = std::move(buffer);
  size_ = size;
  width_ = image.width();
  height_ = image.height();
  channels_ = image.bands();
};

void Image::save(const char *path) const {
  if (!buffer_) {
    throw std::runtime_error("cannot save an empty image");
  }

  auto image = vips::VImage::new_from_memory(
      buffer_.get(), size_, width_, height_, channels_, VIPS_FORMAT_UCHAR);
  image = image.copy(
      vips::VImage::option()->set("interpretation", VIPS_INTERPRETATION_sRGB));
  image.write_to_file(path);
}

int Image::pixelIndex(int x, int y) const {
  return (y * width_ + x) * channels_;
}
int Image::pixelIndex(int x, int y, int width) const {
  return (y * width + x) * channels_;
}

uint8_t *Image::pixelPtr(int x, int y) {
  return buffer_.get() + pixelIndex(x, y);
}

uint8_t *Image::pixelPtr(uint8_t *buffer, int width, int x, int y) {
  return buffer + pixelIndex(x, y, width);
}

PixelView Image::at(int x, int y) {
  int index = pixelIndex(x, y);

  PixelView pixel{buffer_.get() + index, channels_};
  return pixel;
}

Image &Image::grayscale() {
  forEachPixel([](PixelView &pixel) -> void { pixel = pixel.luma(); });
  return *this;
};

Image &Image::flipHorizontal() {
  int xCenter = width_ / 2;
  for (int x = 0; x < xCenter; x++) {
    int xRight = (width_ - 1) - x;
    for (int y = 0; y < height_; y++) {
      swapPixel(at(x, y), at(xRight, y));
    };
  }
  return *this;
};

Image &Image::flipVertical() {
  int yCenter = height_ / 2;
  for (int x = 0; x < width_; x++) {
    for (int y = 0; y < yCenter; y++) {
      swapPixel(at(x, y), at(x, (height_ - 1) - y));
    };
  }
  return *this;
};

Image &Image::rotate90(Rotation direction) {
  int newWidth = height_;
  int newHeight = width_;

  Buffer temp{static_cast<uint8_t *>(g_malloc(size_))};
  bool isCW = direction == Rotation::CW;
  for (int y = 0; y < height_; y++) {
    for (int x = 0; x < width_; x++) {
      int newX = isCW ? height_ - 1 - y : y;
      int newY = isCW ? x : width_ - 1 - x;
      std::copy_n(pixelPtr(x, y), channels_,
                  pixelPtr(temp.get(), newWidth, newX, newY)

      );
    }
  }
  buffer_ = std::move(temp);
  width_ = newWidth;
  height_ = newHeight;
  return *this;
}
