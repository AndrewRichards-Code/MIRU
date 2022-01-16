#pragma once
#include <filesystem>
#include "ARC/src/DynamicLibrary.h"

namespace miru
{
namespace shader_core
{
	std::filesystem::path GetLibraryFullpath_dxil();
	arc::DynamicLibrary::LibraryHandle LoadLibrary_dxil();

	std::filesystem::path GetLibraryFullpath_dxcompiler();
	arc::DynamicLibrary::LibraryHandle LoadLibrary_dxcompiler();
}
}
