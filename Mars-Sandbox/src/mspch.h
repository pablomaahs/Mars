#pragma once

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

#ifndef DISABLE_EASY_PROFILER
#define USING_EASY_PROFILER
#define EASY_PROFILER_STATIC
#endif
#include "easy/profiler.h"

#ifndef DISABLE_OPTICK
#include "optick/src/optick.h"
#endif

#ifdef MSVC
	#include <Windows.h>
#endif