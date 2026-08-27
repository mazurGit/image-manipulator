#pragma once

#include "pixel-view.h"
#include "thread-pool.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <future>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

enum class Rotation { CW, CCW };
enum class ResizeFilter { NearestNeighbor, Bilinear };
enum class CropShape { Square, Circle };
struct PixelPosition {
  int x;
  int y;
};

class Image;

namespace image_io {
Image load(const char *path);
void save(const Image &image, const char *path);
} // namespace image_io

class Image {
  // Buffer ownership
  struct BufferDeleter {
    void operator()(std::uint8_t *buffer) const noexcept;
  };

  using Buffer = std::unique_ptr<std::uint8_t, BufferDeleter>;

public:
  class Row {
  public:
    PixelView operator[](int x) const noexcept {
      return PixelView{data_ + static_cast<std::ptrdiff_t>(x) * channels_,
                       channels_};
    }

  private:
    friend class Image;

    Row(std::uint8_t *data, int channels) noexcept
        : data_{data}, channels_{channels} {}

    std::uint8_t *data_;
    int channels_;
  };

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
  Row operator[](int y) noexcept;
  PixelView at(int y, int x);

  template <typename Func> void forEachPixel(Func func) {
    forEachPixelInRows(0, height_, std::move(func));
  }

  template <typename Func>
  void forEachPixelParallel(ThreadPool &threadPool, Func func) {
    if (height_ <= 0) {
      return;
    }

    const std::size_t rowCount = static_cast<std::size_t>(height_);
    const std::size_t taskCount =
        std::min(rowCount, threadPool.threadCount());
    std::vector<std::future<void>> futures;
    futures.reserve(taskCount);

    try {
      for (std::size_t taskIndex = 0; taskIndex < taskCount; ++taskIndex) {
        const int firstRow =
            static_cast<int>(rowCount * taskIndex / taskCount);
        const int lastRow =
            static_cast<int>(rowCount * (taskIndex + 1) / taskCount);

        futures.emplace_back(threadPool.enqueue(
            [this, firstRow, lastRow, func] {
              forEachPixelInRows(firstRow, lastRow, func);
            }));
      }
    } catch (...) {
      for (auto &future : futures) {
        future.wait();
      }
      throw;
    }

    std::exception_ptr failure;
    for (auto &future : futures) {
      try {
        future.get();
      } catch (...) {
        if (!failure) {
          failure = std::current_exception();
        }
      }
    }
    if (failure) {
      std::rethrow_exception(failure);
    }
  }

  // Transformations
  Image &grayscale();
  Image &invert();
  Image &brightness(int difference);
  Image &contrast(float factor);
  Image &threshold(std::uint8_t value);
  Image &flipHorizontal();
  Image &flipVertical();
  Image &rotate90(Rotation direction);
  Image &resize(int width, int height,
                ResizeFilter filter = ResizeFilter::Bilinear);
  Image &crop(int y, int x, int size, CropShape shape = CropShape::Square);
  Image &blur(int radius);

private:
  template <typename Func>
  void forEachPixelInRows(int firstRow, int lastRow, Func func) {
    for (int y = firstRow; y < lastRow; y++) {
      Row currentRow = (*this)[y];
      for (int x = 0; x < width_; x++) {
        PixelView pixel = currentRow[x];
        if constexpr (std::is_invocable_v<Func &, PixelView, PixelPosition>) {
          func(pixel, PixelPosition{x, y});
        } else {
          func(pixel);
        }
      }
    }
  }
  friend Image image_io::load(const char *path);
  friend void image_io::save(const Image &image, const char *path);

  Buffer buffer_;
  std::size_t size_ = 0;

  // Image geometry
  int width_ = 0;
  int height_ = 0;
  int channels_ = 0;

  // Transformation
  Image &resizeNearest(int width, int height, std::size_t newSize);
  Image &resizeBilinear(int width, int height, std::size_t newSize);
  Image &cropSquare(int y, int x, int size);
  Image &cropCircle(int y, int x, int size);

  void replaceBuffer(Buffer buffer, std::size_t size, int width,
                     int height) noexcept;

  // Indexing helpers
  int pixelIndex(int x, int y) const;
  int pixelIndex(int x, int y, int width) const;
  std::uint8_t *pixelPtr(int x, int y);
  std::uint8_t *pixelPtr(std::uint8_t *buffer, int width, int x, int y);
  Row row(Buffer &buffer, int width, int y) noexcept;
};
