project "Vulkan"
    location "../../vulkan_headers"
    kind "None"
    language "C"

    targetdir   ("../../../bin/%{OutputDir}/%{prj.name}")
    objdir      ("../../../bin-int/%{OutputDir}/%{prj.name}")

    files {
        "../../vulkan_headers/include/**.h",
        "../../vulkan_headers/include/**.hpp"
    }