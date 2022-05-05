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
        "../../imgui/imgui/backends/imgui_impl_opengl3.cpp"
    }

    includedirs {
        "../../imgui",
        "../../glfw/include"
    }

    defines {
        "_CRT_SECURE_NO_WARNINGS"
    }
    
    filter "system:windows"
        systemversion "latest"
        staticruntime "on"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"