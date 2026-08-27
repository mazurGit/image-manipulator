#include "image-io.h"
#include <stdexcept>
#include <utility>
#include <vips/vips8>

Image image_io::load(const char *path) {
  auto decoded = vips::VImage::new_from_file(path)
                     .colourspace(VIPS_INTERPRETATION_sRGB)
                     .cast(VIPS_FORMAT_UCHAR);
  if (decoded.bands() == 3) {
    decoded = decoded.addalpha();
  }

  std::size_t size = 0;
  Image::Buffer buffer{
      static_cast<std::uint8_t *>(decoded.write_to_memory(&size))};

  Image image;
  image.replaceBuffer(std::move(buffer), size, decoded.width(),
                      decoded.height());
  image.channels_ = decoded.bands();
  return image;
}

void image_io::save(const Image &source, const char *path) {
  if (!source.buffer_) {
    throw std::runtime_error("cannot save an empty image");
  }

  auto encoded = vips::VImage::new_from_memory(
      source.buffer_.get(), source.size_, source.width_, source.height_,
      source.channels_, VIPS_FORMAT_UCHAR);
  encoded = encoded.copy(
      vips::VImage::option()->set("interpretation", VIPS_INTERPRETATION_sRGB));
  encoded.write_to_file(path);
}
