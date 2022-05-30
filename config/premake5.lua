workspace "Mars"
    location ".."
    architecture "x86_64"
    startproject "Mars-Sandbox"

    configurations {
        "Debug",
        "Release"
    }

    OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    VULKAN_SDK = os.getenv("VULKAN_SDK")

    LibraryDir = {}
    LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

    Library = {}
    Library["Vulkan"]                   = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
    Library["VulkanUtils"]              = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"
    -- Debug
    Library["ShaderC_Debug"]            = "%{LibraryDir.VulkanSDK_Debug}/shaderc_sharedd.lib"
    Library["SPIRV_Cross_Debug"]        = "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-cored.lib"
    Library["SPIRV_Cross_GLSL_Debug"]   = "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-glsld.lib"
    Library["SPIRV_Tools_Debug"]        = "%{LibraryDir.VulkanSDK_Debug}/SPIRV-Toolsd.lib"
    -- Release
    Library["ShaderC_Release"]          = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
    Library["SPIRV_Cross_Release"]      = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
    Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

    flags {
		"MultiProcessorCompile"
	}

    group "Dependencies"
        include "../vendor/config/glfw"
        include "../vendor/config/glad"
        include "../vendor/config/glm"
        include "../vendor/config/stb"
        include "../vendor/config/imgui"
        include "../vendor/config/easy_profiler"
        include "../vendor/config/optick"
        include "../vendor/config/etc2comp"
        include "../vendor/config/meshoptimizer"
        include "../vendor/config/vulkan_headers"

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
        "../vendor/Etc2Comp/Etc2Comp/EtcTool",
        "../vendor/MeshOptimizer/",
        "../vendor/vulkan_headers/include"
    }

    links {
        "GLFW",
        "Glad",
        "Glm",
        "ImGui",
        "EasyProfiler",
        "Optick",
        "Etc2Comp",
        "Etc2CompTool",
        "MeshOptimizer",
        "%{Library.Vulkan}",
        "%{Library.VulkanUtils}"
    }

    libdirs {
        "%{LibraryDir.VulkanSDK}"
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

        links {
            "../bin/%{OutputDir}/%{prj.name}/assimp-vc142-mtd.lib",
        }

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

        links {
            "../bin/%{OutputDir}/%{prj.name}/assimp-vc142-mt.lib",
        }