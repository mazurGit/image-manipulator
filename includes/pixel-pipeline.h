#pragma once

#include "image.h"
#include "pixel-view.h"
#include <cstdint>
#include <variant>
#include <vector>

class PixelPipeline {
public:
  struct Grayscale {
    void apply(PixelView &pixel) const noexcept;
  };

  struct Invert {
    void apply(PixelView &pixel) const noexcept;
  };

  struct Brightness {
    int difference;

    void apply(PixelView &pixel) const noexcept;
  };

  struct Contrast {
    explicit Contrast(float factor);

    void apply(PixelView &pixel) const noexcept;

    float factor;
  };

  struct Threshold {
    std::uint8_t value;

    void apply(PixelView &pixel) const noexcept;
  };

  using Operation =
      std::variant<Grayscale, Invert, Brightness, Contrast, Threshold>;

  static Image &apply(Image &image, const std::vector<Operation> &operations);
};
