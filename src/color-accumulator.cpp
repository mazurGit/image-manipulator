#include "color-accumulator.h"

void ColorAccumulator::add(const std::uint8_t *pixel, int channels) noexcept {
  for (int channel = 0; channel < channels; ++channel) {
    sums_[channel] += pixel[channel];
  }
  ++count_;
}

void ColorAccumulator::remove(const std::uint8_t *pixel,
                              int channels) noexcept {
  for (int channel = 0; channel < channels; ++channel) {
    sums_[channel] -= pixel[channel];
  }
  --count_;
}

void ColorAccumulator::writeAverage(std::uint8_t *pixel,
                                    int channels) const noexcept {
  for (int channel = 0; channel < channels; ++channel) {
    pixel[channel] = static_cast<std::uint8_t>(sums_[channel] / count_);
  }
}
