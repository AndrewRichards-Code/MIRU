#include "common.h"
#include "GraphicsAPI.h"

using namespace miru;

GraphicsAPI::API GraphicsAPI::s_API = GraphicsAPI::API::UNKNOWN;
bool GraphicsAPI::s_ApiInitialised = false;

bool GraphicsAPI::s_UseSetName = false;
bool GraphicsAPI::s_UseSetNameInitialised = false;

RenderDoc GraphicsAPI::s_RenderDoc = {};

void GraphicsAPI::SetAPI(GraphicsAPI::API api)
{
	if (!s_ApiInitialised)
	{
		s_API = api;
		s_ApiInitialised = true;
	}
}

void GraphicsAPI::SetUseSetName(bool useSetName)
{
	if (!s_UseSetNameInitialised)
	{
		s_UseSetName = useSetName;
		s_UseSetNameInitialised = true;
	}
}

void GraphicsAPI::LoadRenderDoc()
{
	s_RenderDoc = RenderDoc();
}
