#include "image.h"
#include "color-accumulator.h"
#include "pixel-math.h"
#include "pixel-view.h"
#include "timer.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>
#include <vips/vips8>

namespace {

std::size_t checkedBufferSize(int width, int height, int channels) {
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

} // namespace

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

Image &Image::resizeNearest(int width, int height) {
  if (!buffer_) {
    throw std::runtime_error("cannot resize an empty image");
  }
  const std::size_t newSize = checkedBufferSize(width, height, channels_);
  if (width == width_ && height == height_) {
    return *this;
  }

  Buffer temp{static_cast<uint8_t *>(g_malloc(newSize))};

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

  buffer_ = std::move(temp);
  size_ = newSize;
  width_ = width;
  height_ = height;
  return *this;
};

Image &Image::resize(int width, int height, ResizeFilter filter) {
  switch (filter) {
  case ResizeFilter::NearestNeighbor:
    return resizeNearest(width, height);
  case ResizeFilter::Bilinear:
    return resizeBilinear(width, height);
  }
  throw std::invalid_argument("unsupported resize filter");
}

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

  buffer_ = std::move(temp);
  size_ = newSize;
  width_ = size;
  height_ = size;
  return *this;
}

Image &Image::cropCircle(int x, int y, int size) {
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

  buffer_ = std::move(temp);
  width_ = size;
  height_ = size;
  size_ = newSize;
  return *this;
}

Image &Image::resizeBilinear(int width, int height) {
  struct XInfo {
    int left;
    int right;
    float mix;
  };

  if (!buffer_) {
    throw std::runtime_error("cannot resize an empty image");
  }
  const std::size_t newSize = checkedBufferSize(width, height, channels_);
  if (width == width_ && height == height_) {
    return *this;
  }

  const float scaleX = static_cast<float>(width_) / width;
  const float scaleY = static_cast<float>(height_) / height;
  std::vector<XInfo> xTable(width);

  for (int x = 0; x < width; x++) {
    const float sourceX = x * scaleX;
    const int left = static_cast<int>(sourceX);
    xTable[x] = {left, std::min(left + 1, width_ - 1), sourceX - left};
  }

  Buffer temp{static_cast<uint8_t *>(g_malloc(newSize))};
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

  buffer_ = std::move(temp);
  size_ = newSize;
  width_ = width;
  height_ = height;
  return *this;
};

Image &Image::blur(int radius) {
  Timer timer;
  if (!buffer_) {
    throw std::runtime_error("cannot blur an empty image");
  }
  if (radius < 0) {
    throw std::invalid_argument("blur radius cannot be negative");
  }
  if (radius == 0) {
    return *this;
  }

  Buffer temp{static_cast<uint8_t *>(g_malloc(size_))};
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
