OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

VULKAN_SDK_PATH = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["VulkanDir"]                         = "%{VULKAN_SDK_PATH}/Include"

LibraryDir = {}
LibraryDir["VulkanSDK"]                         = "%{VULKAN_SDK_PATH}/Lib"

Library = {}
Library["Vulkan"]                               = "%{LibraryDir.VulkanSDK}/vulkan-1.lib" -- Load as a DLL using Volk

-- Debug
Library["D_SPIRV_Tools"]                        = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"
Library["D_SPIRV_Tools_opt"]                    = "%{LibraryDir.VulkanSDK}/SPIRV-Tools-optd.lib"
Library["D_glslang"]                            = "%{LibraryDir.VulkanSDK}/glslangd.lib"
Library["D_SPIRV"]                              = "%{LibraryDir.VulkanSDK}/SPIRVd.lib"
Library["D_MachineIndependent"]                 = "%{LibraryDir.VulkanSDK}/MachineIndependentd.lib"
Library["D_OGLCompiler"]                        = "%{LibraryDir.VulkanSDK}/OGLCompilerd.lib"
Library["D_OSDependent"]                        = "%{LibraryDir.VulkanSDK}/OSDependentd.lib"
Library["D_GenericCodeGen"]                     = "%{LibraryDir.VulkanSDK}/GenericCodeGend.lib"
Library["D_glslangDefaultResourceLimits"]       = "%{LibraryDir.VulkanSDK}/glslang-default-resource-limitsd.lib"

-- Release
Library["SPIRV_Tools"]                          = "%{LibraryDir.VulkanSDK}/SPIRV-Tools.lib"
Library["SPIRV_Tools_opt"]                      = "%{LibraryDir.VulkanSDK}/SPIRV-Tools-opt.lib"
Library["glslang"]                              = "%{LibraryDir.VulkanSDK}/glslang.lib"
Library["SPIRV"]                                = "%{LibraryDir.VulkanSDK}/SPIRV.lib"
Library["MachineIndependent"]                   = "%{LibraryDir.VulkanSDK}/MachineIndependent.lib"
Library["OGLCompiler"]                          = "%{LibraryDir.VulkanSDK}/OGLCompiler.lib"
Library["OSDependent"]                          = "%{LibraryDir.VulkanSDK}/OSDependent.lib"
Library["GenericCodeGen"]                       = "%{LibraryDir.VulkanSDK}/GenericCodeGen.lib"
Library["glslangDefaultResourceLimits"]         = "%{LibraryDir.VulkanSDK}/glslang-default-resource-limits.lib"