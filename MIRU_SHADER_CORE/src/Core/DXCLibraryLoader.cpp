#include "miru_shader_core_common.h"

#include "DXCLibraryLoader.h"

std::filesystem::path miru::shader_core::GetLibraryFullpath_dxil()
{
	return std::string(PROJECT_DIR) + "redist/dxc/bin/x64/dxil.dll";
}

arc::DynamicLibrary::LibraryHandle miru::shader_core::LoadLibrary_dxil()
{
	return arc::DynamicLibrary::Load(GetLibraryFullpath_dxil().generic_string());
}

std::filesystem::path miru::shader_core::GetLibraryFullpath_dxcompiler()
{
	return std::string(PROJECT_DIR) + "redist/dxc/bin/x64/dxcompiler.dll";
}

arc::DynamicLibrary::LibraryHandle miru::shader_core::LoadLibrary_dxcompiler()
{
	return arc::DynamicLibrary::Load(GetLibraryFullpath_dxcompiler().generic_string());
}