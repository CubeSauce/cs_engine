// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/types.hpp"

#include <cstdlib>
#include <cassert>
#include <cstdio>

#if defined(__ANDROID__)
	// Should break
	#define CS_PLATFORM_ANDROID
	#define CS_PLATFORM_UNIX
	#define CS_BREAK() raise(SIGTRAP)
#elif defined __APPLE__
	#define CS_PLATFORM_APPLE
	#define CS_PLATFORM_UNIX
	#include "signal.h"
	#define CS_BREAK() raise(SIGTRAP)
#elif defined __linux__
	#define CS_PLATFORM_LINUX
	#define CS_PLATFORM_UNIX
	#include "signal.h"
	#define CS_BREAK() raise(SIGTRAP)
#elif defined _WIN64 || defined _WIN32
	#define CS_PLATFORM_WINDOWS
	#define CS_BREAK() __debugbreak()
#else
	//Should break!
	#define CS_PLATFORM_UNKNOWN
#endif

#define CS_ASSERT(cond) if (!(cond)) { CS_BREAK(); }

#ifdef __GNUC__
#define CS_DLL
#define CS_UNIX
#else
#define CS_DLL __declspec(dllexport)
#endif

#define CS_MOVE(x) static_cast<decltype(x)&&>(x)

#define NEARLY_ZERO 1e-4
#define MATH_DEG_TO_RAD(x) float(x * 0.01745329251)
#define MATH_RAD_TO_DEG(x) float(x * 57.2957795131)

#define CS_SHARED_PTR_SHOULD_INVOKE_DESTRUCTOR
//#define CS_MATH_USE_float_PRECISION
