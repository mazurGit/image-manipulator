#pragma once
#include "image.h"
#include "thread-pool.h"
#include <algorithm>
#include <exception>
#include <future>
#include <type_traits>
#include <utility>
#include <vector>

template <typename Func> void Image::forEachPixel(Func func) {
  forEachPixelInRows(0, height_, std::move(func));
}

template <typename Func>
void Image::forEachPixelParallel(ThreadPool &threadPool, Func func) {
  if (height_ <= 0) {
    return;
  }

  const std::size_t rowCount = static_cast<std::size_t>(height_);
  const std::size_t taskCount = std::min(rowCount, threadPool.threadCount());
  std::vector<std::future<void>> futures;
  futures.reserve(taskCount);

  try {
    for (std::size_t taskIndex = 0; taskIndex < taskCount; ++taskIndex) {
      const int firstRow = static_cast<int>(rowCount * taskIndex / taskCount);
      const int lastRow =
          static_cast<int>(rowCount * (taskIndex + 1) / taskCount);

      futures.emplace_back(threadPool.enqueue([this, firstRow, lastRow, func] {
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

template <typename Func>
void Image::forEachPixelInRows(int firstRow, int lastRow, Func func) {
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
