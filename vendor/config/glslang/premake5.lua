project "GlsLang"
    removeconfigurations {
        "Debug",
        "Release"
    }

    location "../../glslang"
    kind "None"

    files {
        "../../glslang/glslang/include/*.h",
        "../../glslang/glslang/Public/*.h",
        "../../glslang/glslang/CInterface/*.h",
        "../../glslang/StandAlone/*.h"
    }

    sysincludedirs {
        "../../glslang/"
    }