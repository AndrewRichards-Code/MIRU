#pragma once
#if defined(MIRU_D3D12)
#define NOMINMAX

//D3D12 Header
#include "../packages/Microsoft.Direct3D.D3D12.1.610.3/build/native/include/d3d12.h"
#include <dxgi1_6.h>
#include "../packages/Microsoft.Direct3D.D3D12.1.610.3/build/native/include/d3dx12/d3dx12.h"

#define MIRU_D3D12_AGILITY_SDK_SET_VERSION_AND_PATH \
extern "C"\
{\
	__declspec(dllexport) extern const unsigned int D3D12SDKVersion = 610;\
	__declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";\
}

//D3D12MemoryAllocator
#define D3D12MA_D3D12_HEADERS_ALREADY_INCLUDED
#include "D3D12MemoryAllocator/include/D3D12MemAlloc.h"

#define MIRU_D3D12_SAFE_RELEASE(x) if((x)) { (x)->Release(); (x) = nullptr; }

//Vulkan HRESULT to string
#include "ARC/src/WindowsErrorHandling.h"
namespace miru
{
	namespace d3d12
	{
		static std::string HRESULTToString(int64_t code)
		{
			return arc::GetLastErrorToString(static_cast<DWORD>(code));
		}
	}
}

#endif