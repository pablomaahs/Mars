project "Stb"
    location "../../stb"
    kind "None"
    language "C"

    targetdir   ("../../../bin/%{OutputDir}/%{prj.name}")
    objdir      ("../../../bin-int/%{OutputDir}/%{prj.name}")

    files {
        "../../stb/stb/*.h"
    }

    includedirs {
        "../../stb/stb"
    }

    defines {
        "_CRT_SECURE_NO_WARNINGS"
    }