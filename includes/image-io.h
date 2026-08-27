#pragma once

#include "image.h"

namespace image_io {

Image load(const char *path);
void save(const Image &image, const char *path);

} // namespace image_io
