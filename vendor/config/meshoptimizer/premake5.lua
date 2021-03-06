project "MeshOptimizer"
    location "../../meshoptimizer/meshoptimizer"
    kind "StaticLib"
    language "C++"

    targetdir   ("../../../bin/%{OutputDir}/%{prj.name}")
    objdir      ("../../../bin-int/%{OutputDir}/%{prj.name}")

    includedirs {
        "../../meshoptimizer/meshoptimizer/src"
    }

    files {
        "../../meshoptimizer/meshoptimizer/src/meshoptimizer.h",
        "../../meshoptimizer/meshoptimizer/src/allocator.cpp",
        "../../meshoptimizer/meshoptimizer/src/clusterizer.cpp",
        "../../meshoptimizer/meshoptimizer/src/indexcodec.cpp",
        "../../meshoptimizer/meshoptimizer/src/indexgenerator.cpp",
        "../../meshoptimizer/meshoptimizer/src/overdrawanalyzer.cpp",
        "../../meshoptimizer/meshoptimizer/src/overdrawoptimizer.cpp",
        "../../meshoptimizer/meshoptimizer/src/simplifier.cpp",
        "../../meshoptimizer/meshoptimizer/src/spatialorder.cpp",
        "../../meshoptimizer/meshoptimizer/src/stripifier.cpp",
        "../../meshoptimizer/meshoptimizer/src/vcacheanalyzer.cpp",
        "../../meshoptimizer/meshoptimizer/src/vcacheoptimizer.cpp",
        "../../meshoptimizer/meshoptimizer/src/vertexcodec.cpp",
        "../../meshoptimizer/meshoptimizer/src/vertexfilter.cpp",
        "../../meshoptimizer/meshoptimizer/src/vfetchanalyzer.cpp",
        "../../meshoptimizer/meshoptimizer/src/vfetchoptimizer.cpp"
    }

    filter "system:windows"
        systemversion "latest"
        staticruntime "On"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"