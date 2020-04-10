#include "miru_core_common.h"
#include "GraphicsAPI.h"

using namespace miru;

GraphicsAPI::API GraphicsAPI::s_API = GraphicsAPI::API::UNKNOWN;
bool GraphicsAPI::s_ApiInitialised = false;

bool GraphicsAPI::s_AllowSetName = false;
bool GraphicsAPI::s_AllowSetNameInitialised = false;

debug::RenderDoc GraphicsAPI::s_RenderDoc = {};

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
	s_RenderDoc = debug::RenderDoc();
}
