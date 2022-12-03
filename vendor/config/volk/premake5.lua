project "Volk"
    location "../../volk"
    kind "None"

    removeconfigurations {
        "Debug",
        "Release"
    }

    files {
        "../../volk/volk.h",
        "../../volk/volk.c"
    }    

    -- location "../../volk"
    -- kind "StaticLib"
    -- language "C++"

    
    -- removeconfigurations {
    --     "Null"
    -- }
    
    -- targetdir   ("../../../bin/%{OutputDir}/%{prj.name}")
    -- objdir      ("../../../bin-int/%{OutputDir}/%{prj.name}")
    
    -- files {
    --     "../../volk/volk.h",
    --     "../../volk/volk.c"
    -- }

    -- includedirs {
    --     "%{IncludeDir.VulkanDir}",
    --     "../../volk/"
    -- }

    -- defines {
    --     "VK_USE_PLATFORM_WIN32_KHR"
    -- }

    -- filter "system:Windows"
    --     systemversion "latest"
    --     staticruntime "on"

    -- filter "configurations:Debug"
    --     runtime "Debug"
    --     optimize "Off"
    --     symbols "On"

    -- filter "configurations:Release"
    --     runtime "Release"
    --     optimize "On" -- "Size" "Speed"
    --     symbols "On" -- "Off"

    -- filter {}