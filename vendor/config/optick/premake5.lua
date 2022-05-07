project "Optick"
    location "../../optick"
    kind "StaticLib"
    language "C++"

    targetdir   ("../../../bin/%{OutputDir}/%{prj.name}")
    objdir      ("../../../bin-int/%{OutputDir}/%{prj.name}")

    includedirs {
        "../../optick/optick/src"
    }

    files {
        "../../optick/optick/src/*.*"
    }

    defines {
        "_CRT_SECURE_NO_WARNINGS",
        "OPTICK_EXPORTS=0",
        "OPTICK_ENABLE_GPU_VULKAN=0",
        "OPTICK_ENABLE_GPU_D3D12=0",
        "OPTICK_ENABLE_GPU=0"
    }

    filter "system:windows"
        systemversion "latest"
        staticruntime "On"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"