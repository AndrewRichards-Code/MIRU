cmake_minimum_required(VERSION 3.27)

project(MIRU)

include("External/ARC/src/CMakeFunctions.cmake")

#External Downloads
file(DOWNLOAD 
	https://raw.githubusercontent.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator/master/include/D3D12MemAlloc.h
	"${CMAKE_CURRENT_SOURCE_DIR}/External/D3D12MemoryAllocator/include/D3D12MemAlloc.h"
)
file(DOWNLOAD 
	https://raw.githubusercontent.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator/master/src/D3D12MemAlloc.cpp
	"${CMAKE_CURRENT_SOURCE_DIR}/External/D3D12MemoryAllocator/src/D3D12MemAlloc.cpp"
)
file(DOWNLOAD 
	https://dist.nuget.org/win-x86-commandline/latest/nuget.exe
	"${CMAKE_CURRENT_SOURCE_DIR}/External/nuget/nuget.exe"
)
file(DOWNLOAD 
	https://raw.githubusercontent.com/baldurk/renderdoc/v1.x/renderdoc/api/app/renderdoc_app.h
	"${CMAKE_CURRENT_SOURCE_DIR}/External/renderdoc/api/app/renderdoc_app.h"
)
file(DOWNLOAD 
	https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
	"${CMAKE_CURRENT_SOURCE_DIR}/External/stb/stb_image.h"
)
file(DOWNLOAD 
	https://raw.githubusercontent.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/master/include/vk_mem_alloc.h
	"${CMAKE_CURRENT_SOURCE_DIR}/External/VulkanMemoryAllocator/include/vk_mem_alloc.h"
)

#MIRU Functions
function(RestoreNuGutPackages)
	find_program(NUGET nuget PATHS "${CMAKE_SOURCE_DIR}/External/nuget/")
	if(NUGET)
		# Copy packages.config file from source to build directory.
		execute_process(COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/packages.config" "${CMAKE_CURRENT_BINARY_DIR}/packages.config")
		# Run Nuget using the packages.config file to install any missing dependencies to the build directory.
		execute_process(COMMAND ${NUGET} restore "${CMAKE_CURRENT_BINARY_DIR}/packages.config" -PackagesDirectory "${CMAKE_BINARY_DIR}/packages")
	endif()
endfunction()

#Project Folders
add_subdirectory(External/ARC)
add_subdirectory(MIRU_CORE)
add_subdirectory(MIRU_SHADER_COMPILER)
add_subdirectory(MIRU_TEST)
#add_subdirectory(MIRU_TEST_UWP)

set_property(DIRECTORY "${CMAKE_SOURCE_DIR}/MIRU_TEST" PROPERTY VS_STARTUP_PROJECT MIRU_TEST)