#include "image.h"
#include "image-io.h"
#include <cstring>
#include <limits>
#include <stdexcept>
#include <utility>
#include <glib.h>

std::size_t Image::checkedBufferSize(int width, int height, int channels) {
  if (width <= 0 || height <= 0) {
    throw std::invalid_argument("image dimensions must be positive");
  }

  const auto widthSize = static_cast<std::size_t>(width);
  const auto heightSize = static_cast<std::size_t>(height);
  const auto channelSize = static_cast<std::size_t>(channels);
  const auto maxSize = std::numeric_limits<std::size_t>::max();

  if (channelSize == 0 || widthSize > maxSize / heightSize / channelSize) {
    throw std::overflow_error("image dimensions are too large");
  }
  return widthSize * heightSize * channelSize;
}

void Image::BufferDeleter::operator()(std::uint8_t *buffer) const noexcept {
  g_free(buffer);
}

void Image::replaceBuffer(Buffer buffer, std::size_t size, int width,
                          int height) noexcept {
  buffer_ = std::move(buffer);
  size_ = size;
  width_ = width;
  height_ = height;
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

void Image::load(const char *path) { *this = image_io::load(path); }

void Image::save(const char *path) const { image_io::save(*this, path); }

int Image::pixelIndex(int x, int y) const { return pixelIndex(x, y, width_); }

int Image::pixelIndex(int x, int y, int width) const {
  return (y * width + x) * channels_;
}

std::uint8_t *Image::pixelPtr(int x, int y) {
  return buffer_.get() + pixelIndex(x, y);
}

std::uint8_t *Image::pixelPtr(std::uint8_t *buffer, int width, int x, int y) {
  return buffer + pixelIndex(x, y, width);
}

Image::Row Image::operator[](int y) noexcept {
  return Row{pixelPtr(0, y), channels_};
}

PixelView Image::at(int y, int x) {
  if (!buffer_) {
    throw std::runtime_error("cannot access an empty image");
  }
  if (y < 0 || y >= height_ || x < 0 || x >= width_) {
    throw std::out_of_range("pixel coordinates are out of range");
  }
  return (*this)[y][x];
}

Image::Row Image::row(Buffer &buffer, int width, int y) noexcept {
  return Row{pixelPtr(buffer.get(), width, 0, y), channels_};
}
