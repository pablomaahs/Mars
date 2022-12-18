#include "mspch.h"

#ifdef GFX_VULKAN
#include "platform/vulkan/msAppVulkan.h"
#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#endif // !VK_NO_PROTOTYPES
#define VOLK_IMPLEMENTATION
#include "volk.h"
#elif GFX_OPENGL
#include "platform/opengl/msAppGL.h"
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

int main()
{
#ifdef GFX_VULKAN
    ms::MsAppVulkan app{ 800, 600, "Vulkan App"};
#elif GFX_OPENGL
    ms::MsAppGL app{ 800, 600, "Open GL App" };
#endif

    try
    {
        app.Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}