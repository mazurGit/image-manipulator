#pragma once

#include <algorithm>
#include <cstdint>

namespace pixel_math {

[[nodiscard]]
constexpr std::uint8_t clampToByte(int value) noexcept {
  return static_cast<std::uint8_t>(std::clamp(value, 0, 255));
}

[[nodiscard]]
constexpr float lerp(float start, float end, float mix) noexcept {
  return start + (end - start) * mix;
}

static constexpr float kLuminanceWeights[3] = {0.299f, 0.587f, 0.114f};
} // namespace pixel_math
