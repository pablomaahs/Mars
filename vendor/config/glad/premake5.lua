project "Glad"
    location "../../glad"
    kind "StaticLib"
    language "C"
    
    targetdir   ("../../../bin/%{OutputDir}/%{prj.name}")
    objdir      ("../../../bin-int/%{OutputDir}/%{prj.name}")

    files {
        "../../glad/include/glad/glad.h",
        "../../glad/include/KHR/khrplatform.h",
        "../../glad/src/glad.c"
    }

    includedirs {
        "../../glad/include"
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