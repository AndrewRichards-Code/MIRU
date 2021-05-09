#pragma once
#if defined(MIRU_D3D12)
//Header and Library
#include <d3d12.h>
#include <dxgi1_6.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

//D3D12MemoryAllocator
#include "D3D12MemoryAllocator/src/D3D12MemAlloc.h"

//DXC Header and Library
#include <d3d12shader.h>
#include <dxcapi.h>
//#pragma comment(lib, "dxc/lib/x64/dxcompiler.lib")

#define MIRU_D3D12_SAFE_RELEASE(x) if((x)) { (x)->Release(); (x) = nullptr; }

#endif