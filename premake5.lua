workspace "Mars"

    architecture "x64"
    startproject "Mars-Sandbox"

    OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

	configurations {
		"Debug",
		"Release"
	}
	
	flags {
		"MultiProcessorCompile"
	}

	-- Include directories relative to OpenGL-Core
	IncludeDir = {}    
	IncludeDir["GLFW"] = "vendor/glfw/include"
	IncludeDir["Glad"] = "vendor/glad/include"
    IncludeDir["Glm"] = "vendor/glm"
	--IncludeDir["ImGui"] = "vendor/imgui"
	--IncludeDir["stb_image"] = "vendor/stb_image"
    --IncludeDir["Vulkan"] = "$(VULKAN_SDK)/include"

    group "Dependencies"
        include "vendor/glfw"
        include "vendor/glad"
    group ""

    include "Mars-Sandbox"