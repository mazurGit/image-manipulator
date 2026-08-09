#pragma once

#include "color.h"
#include <cstdint>
#include <ostream>

class PixelView {
private:
  uint8_t *data_;
  int channels_;

public:
  explicit PixelView(uint8_t *data, int channels);
  uint8_t &getColor(Channel channel);
  const uint8_t &getColor(Channel channel) const;
  uint8_t luma() const;
  PixelView &operator=(uint8_t color);

  template <typename Self, typename Func>
  static void forEachChannelImpl(Self &self, Func func, bool skipAlpha) {
    int count = skipAlpha ? std::min(self.channels_, 3) : self.channels_;
    for (int i = 0; i < count; ++i)
      func(self.data_[i], i);
  }

  template <typename Self>
  auto static &getColorImpl(Self &self, Channel channel) {
    {
      int index = static_cast<int>(channel);
      return self.data_[index];
    }
  }

  template <typename Func>
  void forEachChannel(Func func, bool skipAlpha = true) {
    forEachChannelImpl(*this, func, skipAlpha);
  }

  template <typename Func>
  void forEachChannel(Func func, bool skipAlpha = true) const {
    forEachChannelImpl(*this, func, skipAlpha);
  }

  friend std::ostream &operator<<(std::ostream &os, const PixelView &pixel);

  Color toColor() const noexcept;
};