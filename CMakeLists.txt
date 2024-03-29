cmake_minimum_required(VERSION 3.27)

project(MIRU)

message(STATUS "MIRU")

include("External/ARC/src/CMakeFunctions.cmake")

#External Downloads
#D3D12MemAlloc
if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/External/D3D12MemoryAllocator/include/D3D12MemAlloc.h")
	file(DOWNLOAD 
		https://raw.githubusercontent.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator/master/include/D3D12MemAlloc.h
		"${CMAKE_CURRENT_SOURCE_DIR}/External/D3D12MemoryAllocator/include/D3D12MemAlloc.h"
	)
	file(DOWNLOAD 
		https://raw.githubusercontent.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator/master/src/D3D12MemAlloc.cpp
		"${CMAKE_CURRENT_SOURCE_DIR}/External/D3D12MemoryAllocator/src/D3D12MemAlloc.cpp"
	)
	message(STATUS "Downloaded D3D12MemoryAllocator.")
else()
	message(STATUS "Found D3D12MemoryAllocator.")
endif()

#nuget
if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/External/nuget/nuget.exe")
	file(DOWNLOAD 
		https://dist.nuget.org/win-x86-commandline/latest/nuget.exe
		"${CMAKE_CURRENT_SOURCE_DIR}/External/nuget/nuget.exe"
	)
	message(STATUS "Downloaded nuget.")
else()
	message(STATUS "Found nuget.")
endif()

#renderdoc_app
if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/External/renderdoc/api/app/renderdoc_app.h")
	file(DOWNLOAD 
		https://raw.githubusercontent.com/baldurk/renderdoc/v1.x/renderdoc/api/app/renderdoc_app.h
		"${CMAKE_CURRENT_SOURCE_DIR}/External/renderdoc/api/app/renderdoc_app.h"
	)
	message(STATUS "Downloaded RenderDoc header file.")
else()
	message(STATUS "Found RenderDoc header file.")
endif()

#stb_image
if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/External/stb/stb_image.h")
	file(DOWNLOAD 
		https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
		"${CMAKE_CURRENT_SOURCE_DIR}/External/stb/stb_image.h"
	)
	file(DOWNLOAD 
		https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h
		"${CMAKE_CURRENT_SOURCE_DIR}/External/stb/stb_image_write.h"
	)
	message(STATUS "Downloaded stb_image.")
else()
	message(STATUS "Found stb_image.")
endif()

if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/External/VulkanMemoryAllocator/include/vk_mem_alloc.h")
	file(DOWNLOAD 
		https://raw.githubusercontent.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/master/include/vk_mem_alloc.h
		"${CMAKE_CURRENT_SOURCE_DIR}/External/VulkanMemoryAllocator/include/vk_mem_alloc.h"
	)
	message(STATUS "Downloaded VulkanMemoryAllocator.")
else()
	message(STATUS "Found VulkanMemoryAllocator.")
endif()

#MIRU Functions
function(RestoreNuGutPackages)
	find_program(NUGET nuget PATHS "${CMAKE_CURRENT_SOURCE_DIR}/../External/nuget/")
	if(NUGET)
		# Copy packages.config file from source to build directory.
		execute_process(COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/packages.config" "${CMAKE_CURRENT_BINARY_DIR}/packages.config")
		# Run Nuget using the packages.config file to install any missing dependencies to the build directory.
		execute_process(COMMAND ${NUGET} restore "${CMAKE_CURRENT_BINARY_DIR}/packages.config" -PackagesDirectory "${CMAKE_BINARY_DIR}/packages")
	endif()
endfunction()

option(MIRU_BUILD_MIRU_TEST "Build MIRU_TEST" ON)

#Project Folders
add_subdirectory(External/ARC)
add_subdirectory(MIRU_CORE)
add_subdirectory(MIRU_SHADER_COMPILER)

if (MIRU_BUILD_MIRU_TEST)
	add_subdirectory(MIRU_TEST)
	set_property(DIRECTORY "${CMAKE_SOURCE_DIR}/MIRU_TEST" PROPERTY VS_STARTUP_PROJECT MIRU_TEST)
endif()

#add_subdirectory(MIRU_TEST_UWP)