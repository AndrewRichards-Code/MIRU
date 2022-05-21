#pragma once
#if defined(MIRU_D3D12)
//D3D12 Header
#include <d3d12.h>
#include <dxgi1_6.h>
#include "d3dx12.h"

//D3D12MemoryAllocator
#define D3D12MA_D3D12_HEADERS_ALREADY_INCLUDED
#include "D3D12MemoryAllocator/include/D3D12MemAlloc.h"

#define MIRU_D3D12_SAFE_RELEASE(x) if((x)) { (x)->Release(); (x) = nullptr; }

#endif