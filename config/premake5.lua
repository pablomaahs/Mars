include "Dependencies.lua"

newoption {
    trigger     = "gfxapi",
    value       = "API",
    description = "Choose a particular 3D API for rendering",
    default     = "vulkan",
    category    = "Build Options",
    allowed     = {
       { "opengl", "OpenGL" },
       { "vulkan", "Vulkan" }
    }
 }

print("Building GFX '" .. _OPTIONS["gfxapi"] .. "'...")

workspace "Mars"
    location ".."
    architecture "x86_64"
    startproject "Mars-Sandbox"

    configurations {
        "Null",
        "Debug",
        "Release"
    }

    group "Dependencies"
        include "../vendor/config/glad"
        include "../vendor/config/glfw"
        include "../vendor/config/glm"
        include "../vendor/config/glslang"
        include "../vendor/config/volk"
        include "../vendor/config/easy_profiler"
        include "../vendor/config/meshoptimizer"
        include "../vendor/config/optick"
        include "../vendor/config/etc2comp"
        include "../vendor/config/stb"
        include "../vendor/config/imgui"

    group ""

project "Mars-Sandbox"
    location "../%{prj.name}"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    removeconfigurations {
        "Null"
    }

    targetname  ("Mars")
    targetdir   ("../bin/%{OutputDir}/%{prj.name}")
    objdir      ("../bin-int/%{OutputDir}/%{prj.name}")

    pchheader "mspch.h"
    pchsource "../%{prj.name}/src/mspch.cpp"

    ignoredefaultlibraries {
        "LIBCMT",
        "LIBCMTD"
    }

    files {
        "../%{prj.name}/src/*.h",
        "../%{prj.name}/src/*.cpp",
        "../%{prj.name}/src/platform/common/*.h",
        "../%{prj.name}/src/platform/common/*.cpp",
        "../%{prj.name}/src/platform/common/utils/*.h",
        "../%{prj.name}/src/platform/common/utils/*.cpp",
    }

    includedirs {
        "../%{prj.name}/src/",
        "../vendor/GLFW/include/",
        "../vendor/Glm/",
        "../vendor/easy_profiler/easy_profiler_core/include/",
        "../vendor/meshoptimizer/",
        "../vendor/optick/",
        "../vendor/stb/",
        "../vendor/assimp/include/",
        "../vendor/assimp/build/include/"
    }

    links {
        "GLFW",
        "Glm",
        "EasyProfiler",
        "MeshOptimizer",
        "Optick",
        "Etc2Comp"
    }

    defines {
        --"ENABLE_MESH_OPTIMIZER",
        --"ENABLE_EASY_PROFILER",
        --"ENABLE_OPTICK",
        "GLFW_INCLUDE_NONE",
        "_CRT_SECURE_NO_WARNINGS"
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

    filter "options:gfxapi=opengl"
        files {
            "../%{prj.name}/src/platform/opengl/*.h",
            "../%{prj.name}/src/platform/opengl/*.cpp",
            "../%{prj.name}/src/platform/opengl/utils/*.h",
            "../%{prj.name}/src/platform/opengl/utils/*.cpp",
            "../%{prj.name}/src/platform/opengl/gltrace/*.h",
            "../%{prj.name}/src/platform/opengl/gltrace/*.cpp"
        }

        includedirs {
            "../vendor/Glad/include",
            "../vendor/imgui/imgui"
        }

        links {
            "Glad",
            "ImGui"
        }

        defines {
            "GFX_OPENGL"
        }

    filter { "configurations:Debug", "options:gfxapi=Opengl" }
        links {
            "../bin/%{OutputDir}/%{prj.name}/assimp-vc142-mtd.lib"
        }

    filter { "configurations:Release", "options:gfxapi=Opengl" }
        links {
            "../bin/%{OutputDir}/%{prj.name}/assimp-vc142-mt.lib"
        }

    filter "options:gfxapi=vulkan"
        files {
            "../%{prj.name}/src/platform/vulkan/*.h",
            "../%{prj.name}/src/platform/vulkan/*.cpp",
            "../%{prj.name}/src/platform/vulkan/utils/*.h",
            "../%{prj.name}/src/platform/vulkan/utils/*.cpp"
        }

        includedirs {
            "%{IncludeDir.VulkanDir}",
            "../vendor/glslang/glslang/",
            "../vendor/glslang/StandAlone/",
            "../vendor/volk/"
        }

        links {
            --"Volk"
        }

        defines {
            "GFX_VULKAN",
            "VK_NO_PROTOTYPES",
        }

        linkoptions {
            "/ignore:4099"
        }

    filter { "configurations:Debug", "options:gfxapi=Vulkan" }
        links {
            "%{Library.D_glslang}",
            "%{Library.D_SPIRV}",
            "%{Library.D_MachineIndependent}",
            "%{Library.D_OGLCompiler}",
            "%{Library.D_OSDependent}",
            "%{Library.D_GenericCodeGen}",
            "%{Library.D_glslangDefaultResourceLimits}",
            "%{Library.D_SPIRV_Tools}",
            "%{Library.D_SPIRV_Tools_opt}",
            "../bin/%{OutputDir}/%{prj.name}/assimp-vc142-mtd.lib"
        }

    filter { "configurations:Release", "options:gfxapi=Vulkan" }
        links {
            "%{Library.glslang}",
            "%{Library.SPIRV}",
            "%{Library.MachineIndependent}",
            "%{Library.OGLCompiler}",
            "%{Library.OSDependent}",
            "%{Library.GenericCodeGen}",
            "%{Library.glslangDefaultResourceLimits}",
            "%{Library.SPIRV_Tools}",
            "%{Library.SPIRV_Tools_opt}",
            "../bin/%{OutputDir}/%{prj.name}/assimp-vc142-mt.lib"
        }

    filter {}