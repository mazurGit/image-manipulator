#pragma once

#include <array>
#include <cstdint>

enum class Channel { Red, Green, Blue, Alpha };

class Color {
public:
  Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue,
        std::uint8_t alpha = 255) noexcept
      : channels_{red, green, blue, alpha} {}

  // Indexed access
  std::uint8_t &operator[](std::size_t index) { return channels_[index]; }

  const std::uint8_t &operator[](std::size_t index) const {
    return channels_[index];
  }

  // Named channel access
  std::uint8_t &r() {
    return channels_[static_cast<std::size_t>(Channel::Red)];
  }
  std::uint8_t &g() {
    return channels_[static_cast<std::size_t>(Channel::Green)];
  }
  std::uint8_t &b() {
    return channels_[static_cast<std::size_t>(Channel::Blue)];
  }
  std::uint8_t &a() {
    return channels_[static_cast<std::size_t>(Channel::Alpha)];
  }

private:
  std::array<std::uint8_t, 4> channels_{0, 0, 0, 255};
};
