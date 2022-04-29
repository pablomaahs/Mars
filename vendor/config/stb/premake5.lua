project "Stb"
    location "../../stb"
    kind "None"
    language "C"

    files {
        "../../stb/stb/*.h"
    }

    includedirs {
        "../../stb/stb"
    }

    defines {
        "_CRT_SECURE_NO_WARNINGS"
    }