project "Mars-Sandbox"

    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"

    pchheader "mspch.h"
    pchsource "src/mspch.cpp"

    targetdir   ("../bin/" .. OutputDir .. "/%{prj.name}")
    objdir      ("../bin-int/" .. OutputDir .. "/%{prj.name}")

    files {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs {
        "src",
        "../%{IncludeDir.GLFW}",
        "../%{IncludeDir.Glad}",
        "../%{IncludeDir.Glm}"
        --"../%{IncludeDir.Vulkan}"
    }

    links {
        "GLFW",
        "Glad"
        --"$(VULKAN_SDK)/lib/vulkan-1.lib"
    }

    filter "system:windows"
        systemversion "latest"

        defines {
            "MSCORE_PLATFORM_WINDOWS"
        }

    filter "configurations:Debug"
        defines "MSCORE_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "MSCORE_RELEASE"
        runtime "Release"
        optimize "on"