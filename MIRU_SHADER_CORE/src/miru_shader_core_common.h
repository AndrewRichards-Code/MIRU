#pragma once

//CSTDLIB
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <filesystem>

//DXC Header and Library
#define NOMINMAX
#include "dxc/inc/d3d12shader.h"
#include "dxc/inc/dxcapi.h"

//Spirv-cross Header and Library
#include "SPIRV-Cross/spirv_cross.hpp"
#if defined(_DEBUG)
#pragma comment(lib, "lib/x64/Debug/spirv-cross-core.lib")
#else
#pragma comment(lib, "lib/x64/Release/spirv-cross-core.lib")
#endif

#define MIRU_SHADER_CORE_D3D12_SAFE_RELEASE(x) if((x)) { (x)->Release(); (x) = nullptr; }

//MIRU Debugbreak, Assert and Warn
#include "ARC/src/DebugMacros.h"

#include "ARC/src/Log.h"
#define MIRU_SHADER_CORE_PRINTF ARC_PRINTF

inline arc::Log MiruShaderCoreLog("MIRU_SHADER_CORE");
#ifdef ARC_LOG_INSTANCE
#undef ARC_LOG_INSTANCE
#define ARC_LOG_INSTANCE MiruShaderCoreLog
#endif

//Triggered if x != 0
#define MIRU_SHADER_CORE_ASSERT(x, y) if((x) != 0) { ARC_FATAL(static_cast<int64_t>(x), "%s", y); ARC_ASSERT(false); }

#define MIRU_SHADER_CORE_FATAL(x, y) if((x) != 0) { ARC_FATAL(static_cast<int64_t>(x), "%s", y); }
#define MIRU_SHADER_CORE_ERROR(x, y) if((x) != 0) { ARC_ERROR(static_cast<int64_t>(x), "%s", y); }
#define MIRU_SHADER_CORE_WARN(x, y) if((x) != 0) { ARC_WARN(static_cast<int64_t>(x), "%s", y); }
#define MIRU_SHADER_CORE_INFO(x, y) if((x) != 0) { ARC_INFO(static_cast<int64_t>(x), "%s", y); }