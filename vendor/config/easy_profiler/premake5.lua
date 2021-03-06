project "EasyProfiler"
    location "../../easy_profiler"
    kind "StaticLib"
    language "C++"
    
    targetdir   ("../../../bin/%{OutputDir}/%{prj.name}")
    objdir      ("../../../bin-int/%{OutputDir}/%{prj.name}")

    files {
        "../../easy_profiler/easy_profiler_core/base_block_descriptor.cpp",
        "../../easy_profiler/easy_profiler_core/block.cpp",
        "../../easy_profiler/easy_profiler_core/block_descriptor.cpp",
        "../../easy_profiler/easy_profiler_core/easy_socket.cpp",
        "../../easy_profiler/easy_profiler_core/event_trace_win.cpp",
        "../../easy_profiler/easy_profiler_core/nonscoped_block.cpp",
        "../../easy_profiler/easy_profiler_core/profile_manager.cpp",
        "../../easy_profiler/easy_profiler_core/profiler.cpp",
        "../../easy_profiler/easy_profiler_core/reader.cpp",
        "../../easy_profiler/easy_profiler_core/serialized_block.cpp",
        "../../easy_profiler/easy_profiler_core/thread_storage.cpp",
        "../../easy_profiler/easy_profiler_core/writer.cpp",
        "../../easy_profiler/easy_profiler_core/block_descriptor.h",
        "../../easy_profiler/easy_profiler_core/chunk_allocator.h",
        "../../easy_profiler/easy_profiler_core/current_time.h",
        "../../easy_profiler/easy_profiler_core/current_thread.h",
        "../../easy_profiler/easy_profiler_core/event_trace_win.h",
        "../../easy_profiler/easy_profiler_core/nonscoped_block.h",
        "../../easy_profiler/easy_profiler_core/profile_manager.h",
        "../../easy_profiler/easy_profiler_core/thread_storage.h",
        "../../easy_profiler/easy_profiler_core/spin_lock.h",
        "../../easy_profiler/easy_profiler_core/stack_buffer.h",
        "../../easy_profiler/easy_profiler_core/include/easy/*.h",
        "../../easy_profiler/easy_profiler_core/include/easy/details/*.h"
    }

    includedirs {
        "../../easy_profiler/easy_profiler_core/include/"
    }

    defines {
        "_CRT_SECURE_NO_WARNINGS",
        "_WINSOCK_DEPRECATED_NO_WARNINGS",
        "EASY_PROFILER_VERSION_MAJOR=2",
        "EASY_PROFILER_VERSION_MINOR=1",
        "EASY_PROFILER_VERSION_PATCH=0",
        "EASY_PROFILER_STATIC",
        "BUILD_WITH_EASY_PROFILER",
        "EASY_OPTION_LOG_ENABLED"
    }
    
    filter "system:windows"
        systemversion "latest"
        staticruntime "on"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

project "EasyProfiler_Converter"
    location "../../easy_profiler"
    kind "ConsoleApp"
    language "C++"
    
    targetdir   ("../../../bin/%{OutputDir}/%{prj.name}")
    objdir      ("../../../bin-int/%{OutputDir}/%{prj.name}")

    files {
        "../../easy_profiler/easy_profiler_converter/converter.cpp",
        "../../easy_profiler/easy_profiler_converter/reader.cpp",
        "../../easy_profiler/easy_profiler_converter/converter.h",
        "../../easy_profiler/easy_profiler_converter/reader.h",
        "../../easy_profiler/easy_profiler_converter/main.cpp"
    }

    includedirs {
        "../../easy_profiler/easy_profiler_core/",
        "../../easy_profiler/easy_profiler_core/include/",
        "../../easy_profiler/easy_profiler_converter/include/"
    }

    defines {
        "EASY_PROFILER_STATIC"
    }

    links {
        "EasyProfiler"
    }
    
    filter "system:windows"
        systemversion "latest"
        staticruntime "on"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"