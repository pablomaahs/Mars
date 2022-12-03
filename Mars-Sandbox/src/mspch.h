#pragma once

#pragma warning(disable : 26812)

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // _CRT_SECURE_NO_WARNINGS

#include <assert.h>
#include <stdio.h>

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <thread>

#ifdef GFX_OPENGL
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#endif

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#ifdef ENABLE_EASY_PROFILER
#define USING_EASY_PROFILER
#endif
#define EASY_PROFILER_STATIC
#include "easy/profiler.h"

#ifdef ENABLE_MESH_OPTIMIZER
#include "meshoptimizer/src/meshoptimizer.h"
#endif 

#ifdef ENABLE_OPTICK
#define USE_OPTICK 1
#else
#define USE_OPTICK 0
#endif
#include "optick/src/optick.h"

#ifdef MSVC
	#include <Windows.h>
#endif