project "Glad"

    filter "options:gfxapi=vulkan"
        removeconfigurations {
            "Debug",
            "Release"
        }
    
    filter "options:gfxapi=opengl"
        removeconfigurations {
            "Null"
        }

    filter {}
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

        filter "system:Windows"
            systemversion "latest"
            staticruntime "on"

        filter "configurations:Debug"
            runtime "Debug"
            optimize "Off"
            symbols "On"
    
        filter "configurations:Release"
            runtime "Release"
            optimize "On" -- "Size" "Speed"
            symbols "On" -- "Off"