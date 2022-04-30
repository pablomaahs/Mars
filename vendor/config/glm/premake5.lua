project "Glm"
    location "../../glm"
    kind "StaticLib"
    language "C"
    
    targetdir   ("../../../bin/%{OutputDir}/%{prj.name}")
    objdir      ("../../../bin-int/%{OutputDir}/%{prj.name}")

    files {
        "../../glm/glm/*.hpp",
        "../../glm/glm/detail/*.cpp",
        "../../glm/glm/detail/*.hpp",
        "../../glm/glm/detail/*.inl",
        "../../glm/glm/ext/*.cpp",
        "../../glm/glm/ext/*.hpp",
        "../../glm/glm/ext/*.inl",
        "../../glm/glm/gtc/*.cpp",
        "../../glm/glm/gtc/*.hpp",
        "../../glm/glm/gtc/*.inl",
        "../../glm/glm/gtx/*.cpp",
        "../../glm/glm/gtx/*.hpp",
        "../../glm/glm/gtx/*.inl",
        "../../glm/glm/simd/*.cpp",
        "../../glm/glm/simd/*.hpp",
        "../../glm/glm/simd/*.inl",
        "../../glm/util/glm.natvis"
    }

    includedirs {
        "../../glm"
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