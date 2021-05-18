#pragma once
#if defined(_MSC_VER)
#pragma warning(disable : 26812) //Disables 'Prefered scoped enum' warning C26812
#pragma warning(disable : 26495) //Disables 'Unitialised variable' warning C26495
#pragma warning(disable : 26451) //Disables  'Arithmetic overflow' warning C26451
#endif

//CSTDLIB
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <array>
#include <deque>
#include <map>
#include <memory>
#include <thread>
#include <mutex>
#include <chrono>
#include <assert.h>
#include <cmath>
#include <filesystem>

//PLATORM SYSTEM
#if defined(_WIN64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define NOMINMAX
#include <Windows.h>

#elif defined (__ANDROID__) && (__ANDROID_API__ >= 24)
#include <dlfcn.h>
#include <jni.h>
#include <errno.h>
#include <sys/resource.h>
#include <android/log.h>
#include <android/sensor.h>
#include <android/window.h>
#include <android_native_app_glue.h>
#endif

//GRAPHICS API
#if !defined(MIRU_D3D12) && !defined(MIRU_VULKAN)

#if defined(_WIN64)
#define MIRU_D3D12
#define MIRU_VULKAN
#if defined(MIRU_WIN64_UWP)
#undef MIRU_VULKAN
#endif
#elif defined(__APPLE__)
#define MIRU_METAL //Use Metal?
#elif defined(__linux__) && !defined(__ANDROID__)
#define MIRU_VULKAN
#elif defined (__ANDROID__) && (__ANDROID_API__ >= 24)
#define MIRU_VULKAN
#endif

#endif

//D3D12
#include "directx12/D3D12_Include.h"

//Vulkan
#include "vulkan/VK_Include.h"

//MIRU GraphicsAPI
#include "crossplatform/GraphicsAPI.h"

//MIRU Helpers
#include "ARC/src/StringConversion.h"
#include "ARC/src/DynamicLibrary.h"
#include "ARC/src/ScopeAndRef.h"
namespace miru
{
	auto shrink_uint32_t_to_uint8_t = [](uint32_t a) -> uint8_t
	{
		uint8_t b = 0;
		for (int i = 0; i < 8; i++)
			b += ((0x0F & (a >> i * 4)) / 0xF) << i;
		return b;
	};

	auto alignedSize = [](uint32_t value, uint32_t alignment) -> uint32_t
	{
		return (value + alignment - 1) & ~(alignment - 1);
	};
}

//MIRU Enum Class Bitwise Operators Templates
#define MIRU_ENUM_CLASS_BITWISE_OPERATORS
#if defined(MIRU_ENUM_CLASS_BITWISE_OPERATORS)
#include "ARC/src/EnumClassBitwiseOperators.h"
#endif

//MIRU CPU Heap Allocation tracker
//#define MIRU_CPU_HEAP_ALLOCATION_TRACKER
#if defined(MIRU_CPU_HEAP_ALLOCATION_TRACKER)
void* operator new(size_t size);
void operator delete(void* ptr);
namespace miru
{
	class CPU_HEAP_ALLOC_TRACKER
	{
	public:
		static size_t current_allocated_bytes;
		//static std::map<void*, size_t> current_allocation_map;
	};
}
#endif

//MIRU CPU Profiler
#define MIRU_CPU_PROFILER
#if defined(MIRU_CPU_PROFILER)
#include "ARC/src/DebugMacros.h"
namespace miru
{
	class Timer
	{
		//enum/struct
		struct ProfileDatum
		{
			uint64_t scopeNumber;
			std::string name;
			double duration;
		};

		//Methods
	public:
		Timer(const std::string& name);
		~Timer();

		void Stop();

		static void BeginSession(const std::string& filepath);
		static void EndSession();

		//Members
	private:
		std::string m_Name;
		bool m_Stopped = false;
		std::chrono::time_point<std::chrono::steady_clock> m_StartTP, m_EndTP;

		static uint64_t m_ScopeCount;
		static std::vector<ProfileDatum> m_ProfileData;
		static std::fstream m_File;
	};
}
#if defined(_DEBUG)
#define MIRU_CPU_PROFILE_BEGIN_SESSION(filepath) miru::Timer::BeginSession(filepath);
#define MIRU_CPU_PROFILE_END_SESSION() miru::Timer::EndSession();
#define MIRU_CPU_PROFILE_SCOPE(name) miru::Timer timer##__LINE__(name)
#define MIRU_CPU_PROFILE_FUNCTION() MIRU_CPU_PROFILE_SCOPE(ARC_FUNCSIG)
#else
#define MIRU_CPU_PROFILE_BEGIN_SESSION(filepath);
#define MIRU_CPU_PROFILE_END_SESSION();
#define MIRU_CPU_PROFILE_SCOPE(name)
#define MIRU_CPU_PROFILE_FUNCTION()
#endif
#endif

//MIRU SetName
#define MIRU_ALLOW_API_SETNAME_FN_COMPILE
#include "directx12/D3D12_SetName.h"
#include "vulkan/VK_SetName.h"

//MIRU Debugbreak, Assert and Warn
#include "ARC/src/DebugMacros.h"

#include "ARC/src/Log.h"
#define MIRU_PRINTF ARC_PRINTF

inline arc::Log MiruCoreLog("MIRU_CORE");
#ifdef ARC_LOG_INSTANCE
#undef ARC_LOG_INSTANCE
#define ARC_LOG_INSTANCE MiruCoreLog
#endif

//Triggered if x != 0
#define MIRU_ASSERT(x, y) if((x) != 0) { ARC_FATAL(static_cast<int64_t>(x), "%s", y); ARC_ASSERT(false); }

#define MIRU_FATAL(x, y) if((x) != 0) { ARC_FATAL(static_cast<int64_t>(x), "%s", y); }
#define MIRU_ERROR(x, y) if((x) != 0) { ARC_ERROR(static_cast<int64_t>(x), "%s", y); }
#define MIRU_WARN(x, y) if((x) != 0) { ARC_WARN(static_cast<int64_t>(x), "%s", y); }
#define MIRU_INFO(x, y) if((x) != 0) { ARC_INFO(static_cast<int64_t>(x), "%s", y); }