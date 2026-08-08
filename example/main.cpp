
#include <image.h>
#include <iostream>
#include <pixel-view.h>
#include <vips/vips.h>



int main()
{
    if (VIPS_INIT("playground")) {
        vips_error_exit(nullptr);
    }

    {
        Image image;
        image.load("assets/butterfly.jpeg");

        PixelView pixel = image.at(10, 10);
        std::cout << pixel << '\n';
    }

    vips_shutdown();
    return 0;
}
