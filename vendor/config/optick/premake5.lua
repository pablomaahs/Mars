project "Optick"
    location "../../optick"
    kind "StaticLib"
    language "C++"

    removeconfigurations {
        "Null"
    }

    targetdir   ("../../../bin/%{OutputDir}/%{prj.name}")
    objdir      ("../../../bin-int/%{OutputDir}/%{prj.name}")

    includedirs {
        "%{IncludeDir.VulkanDir}",
        "../../optick/optick/src"
    }

    files {
        "../../optick/optick/src/*.*"
    }

    defines {
        "_CRT_SECURE_NO_WARNINGS",
        "OPTICK_ENABLE_GPU"
    }

    libdirs {
        "%{LibraryDir.VulkanSDK}"
    }

    links { 
        "%{Library.Vulkan}"
    }

    filter "system:Windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        optimize "Off"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "On" -- "Size" "Speed"
        symbols "On" -- "Off"

    filter {}