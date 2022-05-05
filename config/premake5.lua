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
        "../vendor/ImGui/"
    }

    links {
        "GLFW",
        "Glad",
        "Glm",
        "ImGui"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"