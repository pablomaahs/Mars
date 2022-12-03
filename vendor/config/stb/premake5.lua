project "Stb"
    removeconfigurations {
        "Debug",
        "Release"
    }

    location "../../stb"
    kind "None"

    files {
        "../../stb/stb/*.h"
    }