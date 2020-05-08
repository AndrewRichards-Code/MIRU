#include "miru_core_common.h"
#include "GraphicsAPI.h"

using namespace miru;

GraphicsAPI::API GraphicsAPI::s_API = GraphicsAPI::API::UNKNOWN;
bool GraphicsAPI::s_ApiInitialised = false;

bool GraphicsAPI::s_AllowSetName = false;
bool GraphicsAPI::s_AllowSetNameInitialised = false;

std::unique_ptr<debug::RenderDoc> GraphicsAPI::s_RenderDoc;

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

void GraphicsAPI::LoadRenderDoc()
{
	if (GraphicsAPI::IsVulkan())
		s_RenderDoc = std::make_unique<debug::RenderDoc>();
	else
		MIRU_WARN(true, "WARN: GraphicsAPI:API is not VULKAN. Can not LoadRenderDoc.");
}
