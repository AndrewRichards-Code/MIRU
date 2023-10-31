cmake_minimum_required(VERSION 3.27)

# Files
set(BASE_HEADERS 
	"src/base/AccelerationStructure.h"
	"src/base/Allocator.h"
	"src/base/Buffer.h"
	"src/base/CommandPoolBuffer.h"
	"src/base/Context.h"
	"src/base/DescriptorPoolSet.h"
	"src/base/Framebuffer.h"
	"src/base/GraphicsAPI.h"
	"src/base/Image.h"
	"src/base/Pipeline.h"
	"src/base/PipelineHelper.h"
	"src/base/Shader.h"
	"src/base/ShaderBindingTable.h"
	"src/base/Swapchain.h"
	"src/base/Sync.h"
)
set(BASE_CPP_FILES 
	"src/base/AccelerationStructure.cpp"
	"src/base/Allocator.cpp"
	"src/base/Buffer.cpp"
	"src/base/CommandPoolBuffer.cpp"
	"src/base/Context.cpp"
	"src/base/DescriptorPoolSet.cpp"
	"src/base/Framebuffer.cpp"
	"src/base/GraphicsAPI.cpp"
	"src/base/Image.cpp"
	"src/base/Pipeline.cpp"
	"src/base/Shader.cpp"
	"src/base/ShaderBindingTable.cpp"
	"src/base/Sync.cpp"
	"src/base/Swapchain.cpp"
)
set(DEBUG_HEADERS 
	"src/debug/GraphicsDebugger.h"
	"src/debug/Pix.h"
	"src/debug/RenderDoc.h"
)
set(DEBUG_CPP_FILES 
	"src/debug/Pix.cpp"
	"src/debug/RenderDoc.cpp"
)
set(D3D12_HEADERS 
	"src/d3d12/D3D12AccelerationStructure.h"
	"src/d3d12/D3D12Allocator.h"
	"src/d3d12/D3D12Buffer.h"
	"src/d3d12/D3D12CommandPoolBuffer.h"
	"src/d3d12/D3D12Context.h"
	"src/d3d12/D3D12DescriptorPoolSet.h"
	"src/d3d12/D3D12Framebuffer.h"
	"src/d3d12/D3D12Image.h"
	"src/d3d12/D3D12Pipeline.h"
	"src/d3d12/D3D12Shader.h"
	"src/d3d12/D3D12Swapchain.h"
	"src/d3d12/D3D12Sync.h"
	"src/d3d12/D3D12_Include.h"
	"src/d3d12/D3D12_SetName.h"
)
set(D3D12_CPP_FILES
	"src/d3d12/D3D12AccelerationStructure.cpp"
	"src/d3d12/D3D12Allocator.cpp"
	"src/d3d12/D3D12Buffer.cpp"
	"src/d3d12/D3D12CommandPoolBuffer.cpp"
	"src/d3d12/D3D12Context.cpp"
	"src/d3d12/D3D12DescriptorPoolSet.cpp"
	"src/d3d12/D3D12Framebuffer.cpp"
	"src/d3d12/D3D12Image.cpp"
	"src/d3d12/D3D12Pipeline.cpp"
	"src/d3d12/D3D12Shader.cpp"
	"src/d3d12/D3D12Swapchain.cpp"
	"src/d3d12/D3D12Sync.cpp"
)
set(VULKAN_HEADERS 
	"src/vulkan/VKAccelerationStructure.h"
	"src/vulkan/VKAllocator.h"
	"src/vulkan/VKBuffer.h"
	"src/vulkan/VKCommandPoolBuffer.h"
	"src/vulkan/VKContext.h"
	"src/vulkan/VKDescriptorPoolSet.h"
	"src/vulkan/VKFramebuffer.h"
	"src/vulkan/VKImage.h"
	"src/vulkan/VKPipeline.h"
	"src/vulkan/VKShader.h"
	"src/vulkan/VKSwapchain.h"
	"src/vulkan/VKSync.h"
	"src/vulkan/VK_Include.h"
	"src/vulkan/VK_SetName.h"
)
set(VULKAN_CPP_FILES 
	"src/vulkan/VKAccelerationStructure.cpp"
	"src/vulkan/VKAllocator.cpp"
	"src/vulkan/VKBuffer.cpp"
	"src/vulkan/VKCommandPoolBuffer.cpp"
	"src/vulkan/VKContext.cpp"
	"src/vulkan/VKDescriptorPoolSet.cpp"
	"src/vulkan/VKFramebuffer.cpp"
	"src/vulkan/VKImage.cpp"
	"src/vulkan/VKPipeline.cpp"
	"src/vulkan/VKShader.cpp"
	"src/vulkan/VKSwapchain.cpp"
	"src/vulkan/VKSync.cpp"
)
set(MAIN_HEADERS 
	"src/miru_core.h"
	"src/miru_core_common.h"
)
set(MAIN_CPP_FILES 
	"src/miru_core_common.cpp"
)

set(D3D12MA_HEADERS "../External/D3D12MemoryAllocator/include/D3D12MemAlloc.h")
set(D3D12MA_CPP_FILES "../External/D3D12MemoryAllocator/src/D3D12MemAlloc.cpp")
set(VMA_HEADERS "../External/VulkanMemoryAllocator/include/vk_mem_alloc.h")

add_library(MIRU_CORE SHARED)
target_sources(MIRU_CORE PRIVATE ${BASE_CPP_FILES} ${DEBUG_CPP_FILES} ${MAIN_CPP_FILES} ${BASE_HEADERS} ${DEBUG_HEADERS} ${MAIN_HEADERS})

target_link_libraries(MIRU_CORE PRIVATE ARC)
set_target_properties(ARC PROPERTIES FOLDER "External")

source_group("src\\base" FILES ${BASE_CPP_FILES} ${BASE_HEADERS})
source_group("src\\debug" FILES ${DEBUG_CPP_FILES} ${DEBUG_HEADERS})
source_group("src" FILES ${MAIN_CPP_FILES} ${MAIN_HEADERS})

target_include_directories(MIRU_CORE PRIVATE 
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
	"${CMAKE_CURRENT_SOURCE_DIR}/../External"
)
target_compile_definitions(MIRU_CORE PRIVATE MIRU_BUILD_DLL)
target_compile_definitions(MIRU_CORE PRIVATE "PROJECT_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

set(DXCOMPILER_FOUND FALSE)

if (WIN32)
	message(STATUS "D3D12 supported.")
	target_compile_definitions(MIRU_CORE PRIVATE MIRU_D3D12)

	target_sources(MIRU_CORE PRIVATE ${D3D12_CPP_FILES} ${D3D12_HEADERS})
	source_group("src\\d3d12" FILES ${D3D12_CPP_FILES} ${D3D12_HEADERS})
	
	target_include_directories(MIRU_CORE PRIVATE 
		"${CMAKE_CURRENT_SOURCE_DIR}/../External/D3D12MemoryAllocator/include/"
	)
	target_sources(MIRU_CORE PRIVATE ${D3D12MA_CPP_FILES} ${D3D12MA_HEADERS})
	source_group("External\\D3D12MemoryAllocator" FILES ${D3D12MA_CPP_FILES} ${D3D12MA_HEADERS})
	
	target_link_libraries(MIRU_CORE PRIVATE d3d12.lib dxgi.lib)
	
	RestoreNuGutPackages()
	target_sources(MIRU_CORE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/packages.config")
	target_include_directories(MIRU_CORE PRIVATE 
		"${CMAKE_BINARY_DIR}/packages/Microsoft.Direct3D.D3D12.1.610.5/"
		"${CMAKE_BINARY_DIR}/packages/Microsoft.Direct3D.DXC.1.7.2212.23/"
		"${CMAKE_BINARY_DIR}/packages/WinPixEventRuntime.1.0.231030001/")

	CopyToBinaryDirPostBuild(MIRU_CORE "${CMAKE_BINARY_DIR}/packages/Microsoft.Direct3D.D3D12.1.610.5/build/native/bin/x64/D3D12Core.dll" "D3D12/")
	CopyToBinaryDirPostBuild(MIRU_CORE "${CMAKE_BINARY_DIR}/packages/Microsoft.Direct3D.D3D12.1.610.5/build/native/bin/x64/D3D12Core.pdb" "D3D12/")
	CopyToBinaryDirPostBuild(MIRU_CORE "${CMAKE_BINARY_DIR}/packages/Microsoft.Direct3D.D3D12.1.610.5/build/native/bin/x64/d3d12SDKLayers.dll" "D3D12/")
	CopyToBinaryDirPostBuild(MIRU_CORE "${CMAKE_BINARY_DIR}/packages/Microsoft.Direct3D.D3D12.1.610.5/build/native/bin/x64/d3d12SDKLayers.pdb" "D3D12/")

	if (NOT DXCOMPILER_FOUND)
		target_link_directories(MIRU_CORE PRIVATE "${CMAKE_BINARY_DIR}/packages/Microsoft.Direct3D.DXC.1.7.2212.23/build/native/lib/x64/" "")
		target_link_libraries(MIRU_CORE PRIVATE dxcompiler.lib)
		set(DXCOMPILER_FOUND TRUE)
		message(STATUS "dxcompiler found.")

		CopyToBinaryDirPostBuild(MIRU_CORE "${CMAKE_BINARY_DIR}/packages/Microsoft.Direct3D.DXC.1.7.2212.23/build/native/bin/x64/dxcompiler.dll" "")
		CopyToBinaryDirPostBuild(MIRU_CORE "${CMAKE_BINARY_DIR}/packages/Microsoft.Direct3D.DXC.1.7.2212.23/build/native/bin/x64/dxcompiler.pdb" "")
	endif()
	CopyToBinaryDirPostBuild(MIRU_CORE "${CMAKE_BINARY_DIR}/packages/Microsoft.Direct3D.DXC.1.7.2212.23/build/native/bin/x64/dxil.dll" "")
endif()

find_package(Vulkan)
if (Vulkan_FOUND)
	message(STATUS "Vulkan supported.")
	message(STATUS "Vulkan found. Version ${Vulkan_VERSION}")
	target_compile_definitions(MIRU_CORE PRIVATE MIRU_VULKAN)
	set(Vulkan_SDK_DIR "${Vulkan_INCLUDE_DIRS}/../")

	target_sources(MIRU_CORE PRIVATE ${VULKAN_CPP_FILES} ${VULKAN_HEADERS})
	source_group("src\\vulkan" FILES ${VULKAN_CPP_FILES} ${VULKAN_HEADERS})
	
	target_sources(MIRU_CORE PRIVATE ${VMA_HEADERS})
	source_group("External\\VulkanMemoryAllocator" FILES ${VMA_HEADERS})
	
	target_include_directories(MIRU_CORE PRIVATE ${Vulkan_INCLUDE_DIRS})
	target_link_directories(MIRU_CORE PRIVATE "${Vulkan_SDK_DIR}/Lib")
	target_link_libraries(MIRU_CORE PRIVATE vulkan-1.lib optimized spirv-cross-core.lib debug spirv-cross-cored.lib) #Linux?

	if (NOT DXCOMPILER_FOUND)
		target_link_libraries(MIRU_CORE PRIVATE dxcompiler.lib)
		set(DXCOMPILER_FOUND TRUE)
		message(STATUS "dxcompiler found.")
		CopyToBinaryDirPostBuild(MIRU_CORE "${Vulkan_SDK_DIR}/Bin/dxcompiler.dll")
	endif()
endif()
