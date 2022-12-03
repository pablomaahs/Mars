project "ImGui"
    location "../../imgui"
    kind "StaticLib"
    language "C++"

    targetdir   ("../../../bin/%{OutputDir}/%{prj.name}")
    objdir      ("../../../bin-int/%{OutputDir}/%{prj.name}")

    files {
        "../../imgui/imgui/*.h",
        "../../imgui/imgui/*.cpp",
        "../../imgui/imgui/backends/imgui_impl_glfw.cpp",
    }

    includedirs {
        "../../glfw/include",
        "../../imgui/imgui"
    }

    defines {
        "_CRT_SECURE_NO_WARNINGS"
    }

    filter "options:gfxapi=vulkan"
        removeconfigurations {
            "Debug",
            "Release"
        }

        files {
            "../../imgui/imgui/backends/imgui_impl_vulkan.cpp"
        }

        defines {
            "IMGUI_IMPL_VULKAN_NO_PROTOTYPES"
        }

        includedirs {
            "%{IncludeDir.VulkanDir}"
        }

        libdirs {
            "%{LibraryDir.VulkanSDK}"
        }
    
        links { 
            "%{Library.Vulkan}"
        }

    filter "options:gfxapi=opengl"
        removeconfigurations {
            "Null"
        }

        files {
            "../../imgui/imgui/backends/imgui_impl_opengl3.cpp"
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