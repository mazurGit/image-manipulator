
#include <image.h>
#include <pixel-view.h>
#include <vips/vips.h>

int main() {
  if (VIPS_INIT("playground")) {
    vips_error_exit(nullptr);
  }

  {
    Image image;
    image.load("assets/butterfly.jpeg");
    image.resize(100, 200, ResizeFilter::Bilinear);
    image.save("assets/out.jpeg");
  }

  vips_shutdown();
  return 0;
}
