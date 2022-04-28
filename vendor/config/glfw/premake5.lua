project "GLFW"
    location "../../glfw"
    kind "StaticLib"
    language "C"

    targetdir   ("../../../bin/%{OutputDir}/%{prj.name}")
    objdir      ("../../../bin-int/%{OutputDir}/%{prj.name}")

    includedirs {
        "../../glfw/include"
    }

    files {
        "../../glfw/src/internal.h",
        "../../glfw/src/mappings.h",
        "../../glfw/include/glfw/**.h",
        "../../glfw/src/context.c",
        "../../glfw/src/init.c",
        "../../glfw/src/input.c",
        "../../glfw/src/monitor.c",
        "../../glfw/src/vulkan.c",
        "../../glfw/src/window.c"
    }

    filter "system:windows"
        systemversion "latest"
        staticruntime "On"

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

        defines { 
            "_GLFW_WIN32",
            --"_GLFW_BUILD_DLL",
            "_CRT_SECURE_NO_WARNINGS"
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"