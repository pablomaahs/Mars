project "Glm"
    location "../../glm"
    kind "StaticLib"
    language "C"

    removeconfigurations {
        "Null"
    }

    targetdir   ("../../../bin/%{OutputDir}/%{prj.name}")
    objdir      ("../../../bin-int/%{OutputDir}/%{prj.name}")

    includedirs {
        "../../glm"
    }

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

    filter "system:windows"
        systemversion "latest"
        staticruntime "On"

        defines {
            "_CRT_SECURE_NO_WARNINGS"
        }

    filter "configurations:Debug"
        runtime "Debug"
        optimize "Off"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "On" -- "Size" "Speed"
        symbols "On" -- "Off"