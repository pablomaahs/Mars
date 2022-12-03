project "GLFW"
    location "../../glfw"
    kind "StaticLib"
    language "C"

    removeconfigurations {
        "Null"
    }

    targetdir   ("../../../bin/%{OutputDir}/%{prj.name}")
    objdir      ("../../../bin-int/%{OutputDir}/%{prj.name}")

    includedirs {
        "../../glfw/include"
    }

    files {
        "../../glfw/include/glfw/**.h",
        "../../glfw/src/internal.h",
        "../../glfw/src/mappings.h",
        "../../glfw/src/context.c",
        "../../glfw/src/init.c",
        "../../glfw/src/input.c",
        "../../glfw/src/monitor.c",
        "../../glfw/src/vulkan.c",
        "../../glfw/src/window.c"
    }

    filter "system:Windows"
        systemversion "latest"
        staticruntime "On"

        defines { 
            "_GLFW_WIN32",
            "_CRT_SECURE_NO_WARNINGS"
        }

        files {
            "../../glfw/src/win32_platform.h",
            "../../glfw/src/win32_joystick.h",
            "../../glfw/src/wgl_context.h",
            "../../glfw/src/egl_context.h",
            "../../glfw/src/osmesa_context.h",
            "../../glfw/src/win32_init.c",
            "../../glfw/src/win32_joystick.c",
            "../../glfw/src/win32_monitor.c",
            "../../glfw/src/win32_time.c",
            "../../glfw/src/win32_thread.c",
            "../../glfw/src/win32_window.c",
            "../../glfw/src/wgl_context.c",
            "../../glfw/src/egl_context.c",
            "../../glfw/src/osmesa_context.c"
        }

    filter "configurations:Debug"
        runtime "Debug"
        optimize "Off"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "On" -- "Size" "Speed"
        symbols "On" -- "Off"