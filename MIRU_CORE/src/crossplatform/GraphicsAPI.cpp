#include "miru_core_common.h"
#include "GraphicsAPI.h"

using namespace miru;
using namespace crossplatform;

GraphicsAPI::API GraphicsAPI::s_API = GraphicsAPI::API::UNKNOWN;
bool GraphicsAPI::s_ApiInitialised = false;

bool GraphicsAPI::s_AllowSetName = false;
bool GraphicsAPI::s_AllowSetNameInitialised = false;

std::shared_ptr<debug::GraphicsDebugger> GraphicsAPI::s_GraphicsDebugger;

void GraphicsAPI::SetAPI(GraphicsAPI::API api)
{
	if (!s_ApiInitialised)
	{
		s_API = api;
		s_ApiInitialised = true;
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
	if (!s_AllowSetNameInitialised)
	{
		switch (debugger)
		{
		default:
		case debug::GraphicsDebugger::DebuggerType::NONE:
		{
			s_GraphicsDebugger = nullptr;
		}
		case debug::GraphicsDebugger::DebuggerType::PIX:
		{
			if (GraphicsAPI::IsD3D12())
				s_GraphicsDebugger = std::make_shared<debug::Pix>();
			else
				s_GraphicsDebugger = std::make_shared<debug::RenderDoc>();
		}
		case debug::GraphicsDebugger::DebuggerType::RENDER_DOC:
		{
			s_GraphicsDebugger = std::make_shared<debug::RenderDoc>();
		}
		}
	}

}
