project "Etc2Comp"
    location "../../etc2comp/etc2comp"
    kind "StaticLib"
    language "C++"
    
    targetdir   ("../../../bin/%{OutputDir}/%{prj.name}")
    objdir      ("../../../bin-int/%{OutputDir}/%{prj.name}")

    files {
        "../../etc2comp/etc2comp/EtcLib/Etc/*.h",
        "../../etc2comp/etc2comp/EtcLib/EtcCodec/*.h",
        "../../etc2comp/etc2comp/EtcLib/Etc/*.cpp",
        "../../etc2comp/etc2comp/EtcLib/EtcCodec/*.cpp"
    }

    includedirs {
        "../../etc2comp/etc2comp/EtcLib/Etc",
        "../../etc2comp/etc2comp/EtcLib/EtcCodec"
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

    filter {}

project "Etc2CompTool"
    location "../../etc2comp/etc2comp"
    kind "ConsoleApp"
    language "C++"

    characterset ("MBCS")
    
    targetdir   ("../../../bin/%{OutputDir}/%{prj.name}")
    objdir      ("../../../bin-int/%{OutputDir}/%{prj.name}")

    files {
        "../../etc2comp/etc2comp/EtcTool/*.h",
        "../../etc2comp/etc2comp/EtcTool/*.cpp",
        "../../etc2comp/etc2comp/third_party/lodepng/*.h",
        "../../etc2comp/etc2comp/third_party/lodepng/*.cpp"
    }

    includedirs {
        "../../etc2comp/etc2comp/EtcLib/Etc",
        "../../etc2comp/etc2comp/EtcLib/EtcCodec",
        "../../etc2comp/etc2comp/third_party/lodepng",
        "../../etc2comp/etc2comp/EtcTool/"
    }

    links {
        "Etc2Comp"
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

    filter {}