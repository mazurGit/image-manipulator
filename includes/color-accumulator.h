#pragma once

#include <array>
#include <cstdint>

class ColorAccumulator {
public:
  void add(const std::uint8_t *pixel, int channels) noexcept;
  void remove(const std::uint8_t *pixel, int channels) noexcept;
  void writeAverage(std::uint8_t *pixel, int channels) const noexcept;

private:
  std::array<std::uint64_t, 4> sums_{};
  std::uint64_t count_{};
};
