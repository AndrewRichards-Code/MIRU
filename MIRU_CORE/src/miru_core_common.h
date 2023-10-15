#pragma once

#if defined(_MSC_VER)
#pragma warning(disable : 26812) //Disables 'Prefered scoped enum' warning C26812
#pragma warning(disable : 26495) //Disables 'Unitialised variable' warning C26495
#pragma warning(disable : 26451) //Disables  'Arithmetic overflow' warning C26451
#pragma warning(disable : 4251)  //Disables 'Needs  dll-interface' warning C4251
#endif

//CSTDLIB
#include <string>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>

//PLATORM SYSTEM HELPERS
#if defined(_WIN64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#elif defined (__ANDROID__) && (__ANDROID_API__ >= 24)
#define memcpy_s(dst, dstSize, src, srcSize) memcpy(dst, src, srcSize)
#endif

//GRAPHICS API
#if !defined(MIRU_D3D12) && !defined(MIRU_VULKAN)

#if defined(_WIN64)
#define MIRU_D3D12
#define MIRU_VULKAN
#if defined(MIRU_WIN64_UWP)
#undef MIRU_VULKAN
#define ARC_WIN64_UWP
#undef WINAPI_FAMILY
#define WINAPI_FAMILY WINAPI_FAMILY_PC_APP
#endif
#elif defined(__APPLE__)
#define MIRU_METAL //Use Metal?
#elif defined(__linux__) && !defined(__ANDROID__)
#define MIRU_VULKAN
#elif defined (__ANDROID__) && (__ANDROID_API__ >= 24)
#define MIRU_VULKAN
#endif

#endif

//MIRU API
#include "ARC/src/ExportAttributes.h"
#ifdef MIRU_BUILD_DLL
#define MIRU_API ARC_EXPORT
#else
#define MIRU_API ARC_IMPORT
#endif

//MIRU GraphicsAPI
#include "base/GraphicsAPI.h"

//MIRU Helpers
#include "ARC/src/StringConversion.h"
#include "ARC/src/DynamicLibrary.h"
#include "ARC/src/ScopeAndRef.h"
#include "ARC/src/Helpers.h"

#include "ARC/External/magic_enum/magic_enum.hpp"

//MIRU Class Forward Decalaration and Ref types
#define MIRU_FORWARD_DECLARE_CLASS_AND_REF(_class) class _class; typedef Ref<_class> _class##Ref

namespace miru::base
{
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(AccelerationStructureBuildInfo);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(AccelerationStructure);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Allocator);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Buffer);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(BufferView);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(CommandPool);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(CommandBuffer);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Context);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(DescriptorPool);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(DescriptorSetLayout);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(DescriptorSet);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Framebuffer);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Image);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(ImageView);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Sampler);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(RenderPass);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Pipeline);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Shader);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(ShaderBindingTable);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Swapchain);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Fence);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Semaphore);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Event);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Barrier);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Barrier2);
}
namespace miru::d3d12
{
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(AccelerationStructureBuildInfo);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(AccelerationStructure);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Allocator);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Buffer);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(BufferView);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(CommandPool);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(CommandBuffer);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Context);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(DescriptorPool);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(DescriptorSetLayout);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(DescriptorSet);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Framebuffer);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Image);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(ImageView);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Sampler);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(RenderPass);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Pipeline);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Shader);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(ShaderBindingTable);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Swapchain);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Fence);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Semaphore);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Event);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Barrier);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Barrier2);
}
namespace miru::vulkan
{
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(AccelerationStructureBuildInfo);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(AccelerationStructure);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Allocator);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Buffer);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(BufferView);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(CommandPool);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(CommandBuffer);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Context);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(DescriptorPool);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(DescriptorSetLayout);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(DescriptorSet);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Framebuffer);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Image);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(ImageView);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Sampler);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(RenderPass);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Pipeline);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Shader);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(ShaderBindingTable);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Swapchain);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Fence);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Semaphore);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Event);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Barrier);
	MIRU_FORWARD_DECLARE_CLASS_AND_REF(Barrier2);
}

//MIRU Enum Class Bitwise Operators Templates
#include "ARC/src/EnumClassBitwiseOperators.h"

//MIRU CPU Profiler
#define MIRU_CPU_PROFILER
#if defined(MIRU_CPU_PROFILER)
namespace miru
{
	class MIRU_API Timer
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
#define MIRU_CPU_PROFILE_SCOPE_LINE2(name, line) miru::Timer timer##line(name)
#define MIRU_CPU_PROFILE_SCOPE_LINE(name, line) MIRU_CPU_PROFILE_SCOPE_LINE2(name, line)
#define MIRU_CPU_PROFILE_SCOPE(name) MIRU_CPU_PROFILE_SCOPE_LINE(name, __LINE__)
#define MIRU_CPU_PROFILE_FUNCTION() MIRU_CPU_PROFILE_SCOPE(ARC_FUNCSIG)
#else
#define MIRU_CPU_PROFILE_BEGIN_SESSION(filepath);
#define MIRU_CPU_PROFILE_END_SESSION();
#define MIRU_CPU_PROFILE_SCOPE(name)
#define MIRU_CPU_PROFILE_FUNCTION()
#endif
#endif

//MIRU Debugbreak, Assert and Warn
#include "ARC/src/DebugMacros.h"

#include "ARC/src/Log.h"

inline arc::Log MiruCoreLog("MIRU_CORE");
#ifdef ARC_LOG_INSTANCE
#undef ARC_LOG_INSTANCE
#define ARC_LOG_INSTANCE MiruCoreLog
#endif

#define MIRU_ASSERT(x) ARC_ASSERT((x));

#define MIRU_FATAL(x, y) if((x) != 0) { ARC_FATAL(static_cast<int64_t>(x), "%s", y); ARC_ASSERT(false); }
#define MIRU_ERROR(x, y) if((x) != 0) { ARC_ERROR(static_cast<int64_t>(x), "%s", y); }
#define MIRU_WARN(x, y) if((x) != 0) { ARC_WARN(static_cast<int64_t>(x), "%s", y); }
#define MIRU_INFO(x, y) if((x) != 0) { ARC_INFO(static_cast<int64_t>(x), "%s", y); }