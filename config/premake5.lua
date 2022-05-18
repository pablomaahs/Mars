workspace "Mars"
    location ".."
    architecture "x86_64"
    startproject "Mars-Sandbox"

    configurations {
        "Debug",
        "Release"
    }

    OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    group "Dependencies"
        include "../vendor/config/glfw"
        include "../vendor/config/glad"
        include "../vendor/config/glm"
        include "../vendor/config/stb"
        include "../vendor/config/imgui"
        include "../vendor/config/easy_profiler"
        include "../vendor/config/optick"
        include "../vendor/config/etc2comp"

    group ""

project "Mars-Sandbox"
location "../%{prj.name}"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir   ("../bin/%{OutputDir}/%{prj.name}")
    objdir      ("../bin-int/%{OutputDir}/%{prj.name}")

    pchheader "mspch.h"
    pchsource "../%{prj.name}/src/mspch.cpp"

    files {
        "../%{prj.name}/src/**.h",
        "../%{prj.name}/src/**.cpp"
    }

    includedirs {
        "../%{prj.name}/src",
        "../vendor/GLFW/include",
        "../vendor/Glad/include",
        "../vendor/Glm/",
        "../vendor/Stb/",
        "../vendor/ImGui/",
        "../vendor/Easy_Profiler/easy_profiler_core/include",
        "../vendor/Optick",
        "../vendor/Assimp/include",
        "../vendor/Assimp/build/include",
        "../vendor/Etc2Comp/Etc2Comp/EtcLib/Etc",
        "../vendor/Etc2Comp/Etc2Comp/EtcLib/EtcCodec",
        "../vendor/Etc2Comp/Etc2Comp/EtcTool"
    }

    links {
        "GLFW",
        "Glad",
        "Glm",
        "ImGui",
        "EasyProfiler",
        "Optick",
        "../bin/%{OutputDir}/%{prj.name}/assimp-vc142-mt.lib",
        "Etc2Comp",
        "Etc2CompTool"
    }

    defines {
        "DISABLE_EASY_PROFILER",
        "DISABLE_OPTICK"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"