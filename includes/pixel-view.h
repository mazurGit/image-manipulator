#pragma once

#include "channel.h"
#include <algorithm>
#include <cstdint>
#include <ostream>

class PixelView {
public:
  explicit PixelView(std::uint8_t *data, int channels);

  // Color access
  std::uint8_t &getColor(Channel channel);
  const std::uint8_t &getColor(Channel channel) const;
  std::uint8_t luma() const;
  PixelView &invert() noexcept;
  PixelView &adjustBrightness(int difference) noexcept;
  PixelView &adjustContrast(float factor) noexcept;
  PixelView &applyThreshold(std::uint8_t threshold) noexcept;

  // Channel iteration
  template <typename Func>
  void forEachChannel(Func func, bool skipAlpha = true) {
    forEachChannelImpl(*this, func, skipAlpha);
  }

  template <typename Func>
  void forEachChannel(Func func, bool skipAlpha = true) const {
    forEachChannelImpl(*this, func, skipAlpha);
  }

  // Operators
  PixelView &operator=(std::uint8_t color);
  PixelView &operator=(const PixelView &other) noexcept;

  friend void swapPixel(PixelView a, PixelView b) noexcept;
  friend std::ostream &operator<<(std::ostream &os, const PixelView &pixel);

private:
  std::uint8_t *data_;
  int channels_;

  template <typename Self, typename Func>
  static void forEachChannelImpl(Self &self, Func func, bool skipAlpha) {
    int count = skipAlpha ? std::min(self.channels_, 3) : self.channels_;
    for (int i = 0; i < count; ++i) {
      func(self.data_[i], i);
    }
  }

  template <typename Self>
  static auto &getColorImpl(Self &self, Channel channel) {
    int index = static_cast<int>(channel);
    return self.data_[index];
  }
};
