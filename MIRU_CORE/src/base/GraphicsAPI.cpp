#include "miru_core_common.h"
#include "GraphicsAPI.h"

#if defined(MIRU_D3D12)
#include "d3d12/D3D12_Include.h"
#endif
#if defined(MIRU_VULKAN)
#include "vulkan/VK_Include.h"
#endif

using namespace miru;
using namespace base;

GraphicsAPI::API GraphicsAPI::s_API = GraphicsAPI::API::UNKNOWN;
bool GraphicsAPI::s_ApiInitialised = false;

bool GraphicsAPI::s_AllowSetName = false;
bool GraphicsAPI::s_AllowSetNameInitialised = false;

std::shared_ptr<debug::GraphicsDebugger> GraphicsAPI::s_GraphicsDebugger;

void GraphicsAPI::SetAPI(GraphicsAPI::API api, bool forceOverride)
{
	if (!s_ApiInitialised || forceOverride)
	{
		s_API = api;
		s_ApiInitialised = true;
	}

	if (GraphicsAPI::IsD3D12())
	{
	#if defined(MIRU_D3D12)
		MiruCoreLog.SetErrorCodeToStringFunction(d3d12::HRESULTToString);
	#endif
	}
	else
	{
	#if defined(MIRU_VULKAN)
		MiruCoreLog.SetErrorCodeToStringFunction(vulkan::VkResultToString);
	#endif
	}
}

void GraphicsAPI::AllowSetName(bool allowSetName)
{
	if (!s_AllowSetNameInitialised)
	{
		s_AllowSetName = allowSetName;
		s_AllowSetNameInitialised = true;
	}
}

void GraphicsAPI::LoadGraphicsDebugger(debug::GraphicsDebugger::DebuggerType debugger)
{
	switch (debugger)
	{
	default:
	case debug::GraphicsDebugger::DebuggerType::NONE:
	{
		s_GraphicsDebugger = nullptr;
		break;
	}
	case debug::GraphicsDebugger::DebuggerType::PIX:
	{
		if (GraphicsAPI::IsD3D12())
			s_GraphicsDebugger = std::make_shared<debug::Pix>();
		else
			s_GraphicsDebugger = std::make_shared<debug::RenderDoc>();
		
		break;
	}
	case debug::GraphicsDebugger::DebuggerType::RENDER_DOC:
	{
		s_GraphicsDebugger = std::make_shared<debug::RenderDoc>();
		break;
	}
	}
}
