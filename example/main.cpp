
#include <image.h>
#include <pixel-view.h>
#include <vips/vips.h>

int main() {
  if (VIPS_INIT("playground")) {
    vips_error_exit(nullptr);
  }

  {
    Image image;
    image.load("assets/butterfly.png");
    image.crop(100, 100, 80, CropShape::Circle);
    image.save("assets/out.jpeg");
  }

  vips_shutdown();
  return 0;
}
