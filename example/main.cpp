
#include <image.h>
#include <pixel-pipeline.h>
#include <pixel-view.h>
#include <vips/vips.h>

int main() {
  if (VIPS_INIT("playground")) {
    vips_error_exit(nullptr);
  }

  {
    Image image;
    image.load("assets/butterfly.jpeg");
    PixelPipeline::apply(image, {PixelPipeline::Brightness{20},
                                 PixelPipeline::Contrast{1.1f},
                                 PixelPipeline::Invert{}});
    image.blur(3);
    image.save("assets/out.jpeg");
  }

  vips_shutdown();
  return 0;
}
