
#include <image.h>
#include <image-io.h>
#include <pixel-pipeline.h>
#include <pixel-view.h>
#include <vips/vips.h>

int main() {
  if (VIPS_INIT("playground")) {
    vips_error_exit(nullptr);
  }

  {
    Image image = image_io::load("assets/butterfly.jpeg");
    PixelPipeline::apply(image, {PixelPipeline::Brightness{100}});
    // image.resize(8000, 4000);
    image_io::save(image, "assets/out.jpeg");
  }

  vips_shutdown();
  return 0;
}
