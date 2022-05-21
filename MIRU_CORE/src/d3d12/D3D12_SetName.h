#pragma once
#if defined(MIRU_D3D12)

#include "ARC/src/StringConversion.h"

namespace miru
{
namespace d3d12
{
	typedef HRESULT(WINAPI* PFN_PIXBeginEventOnCommandList)(ID3D12GraphicsCommandList*, UINT64, _In_ PCSTR);
	typedef HRESULT(WINAPI* PFN_PIXEndEventOnCommandList)(ID3D12GraphicsCommandList*);
	inline PFN_PIXBeginEventOnCommandList PIXBeginEventOnCommandList;
	inline PFN_PIXEndEventOnCommandList PIXEndEventOnCommandList;

#if defined(MIRU_ALLOW_API_SETNAME_FN_COMPILE) && defined(_DEBUG)
	static inline void D3D12SetName(void* object, const std::string& name)
	{
		if (!base::GraphicsAPI::IsSetNameAllowed())
			return;

		std::wstring w_name = arc::ToWString(name);

		reinterpret_cast<ID3D12Object*>(object)->SetName(w_name.c_str());
	}
#else
	static inline void D3D12SetName(void* object, const std::string& name) { return; }
#endif
}
}
#endif