cmake_minimum_required( VERSION 3.17 )

set(MIRU_VULKAN_SDK_DIR $ENV{VULKAN_SDK} CACHE STRING "Directory to VulkanSDK" )

project( MIRU )

#MIRU_SHADER_COMPILER
set( src_h
    MIRU_SHADER_COMPILER/src/BuildCSO.h
    MIRU_SHADER_COMPILER/src/BuildSPV.h
    MIRU_SHADER_COMPILER/src/ErrorCodes.h
    MIRU_SHADER_COMPILER/src/MSCDocumentation.h
)
set( src_cpp
    MIRU_SHADER_COMPILER/src/Main.cpp
)

set( src_public
    ${src_h}
    ${src_cpp}
)

set( CMAKE_GENERATOR_PLATFORM Visual Studio 16 2019 Win64)
set( CMAKE_GENERATOR_TOOLSET v142, host=x64)
set( CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/MIRU_SHADER_COMPILER/exe/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>")

add_executable(MIRU_SHADER_COMPILER ${src_public})
target_compile_features(MIRU_SHADER_COMPILER PUBLIC cxx_std_17)

#MIRU_CORE
set( src_crossplatform_h
	MIRU_CORE/src/crossplatform/Allocator.h
    MIRU_CORE/src/crossplatform/Buffer.h
    MIRU_CORE/src/crossplatform/CommandPoolBuffer.h
    MIRU_CORE/src/crossplatform/Context.h
    MIRU_CORE/src/crossplatform/DescriptorPoolSet.h
    MIRU_CORE/src/crossplatform/Framebuffer.h
    MIRU_CORE/src/crossplatform/GraphicsAPI.h
    MIRU_CORE/src/crossplatform/Image.h
    MIRU_CORE/src/crossplatform/Pipeline.h
    MIRU_CORE/src/crossplatform/PipelineHelper.h
    MIRU_CORE/src/crossplatform/Shader.h
    MIRU_CORE/src/crossplatform/Swapchain.h
    MIRU_CORE/src/crossplatform/Sync.h
)
set( src_crossplatform_cpp
    MIRU_CORE/src/crossplatform/Allocator.cpp
    MIRU_CORE/src/crossplatform/Buffer.cpp
    MIRU_CORE/src/crossplatform/CommandPoolBuffer.cpp
    MIRU_CORE/src/crossplatform/Context.cpp
    MIRU_CORE/src/crossplatform/DescriptorPoolSet.cpp
    MIRU_CORE/src/crossplatform/Framebuffer.cpp
    MIRU_CORE/src/crossplatform/GraphicsAPI.cpp
    MIRU_CORE/src/crossplatform/Image.cpp
    MIRU_CORE/src/crossplatform/Pipeline.cpp
    MIRU_CORE/src/crossplatform/Shader.cpp
    MIRU_CORE/src/crossplatform/Sync.cpp
    MIRU_CORE/src/crossplatform/Swapchain.cpp
)

set( src_debug_h 
    MIRU_CORE/src/debug/GraphicsDebugger.h
    MIRU_CORE/src/debug/Pix.h
    MIRU_CORE/src/debug/RenderDoc.h
)
set( src_debug_cpp 
    MIRU_CORE/src/debug/Pix.cpp
    MIRU_CORE/src/debug/RenderDoc.cpp
)

set( src_directx12_h
    MIRU_CORE/redist/D3D12MemoryAllocator/src/D3D12MemAlloc.h
    MIRU_CORE/src/directx12/D3D12Allocator.h
    MIRU_CORE/src/directx12/D3D12Buffer.h
    MIRU_CORE/src/directx12/D3D12CommandPoolBuffer.h
    MIRU_CORE/src/directx12/D3D12Context.h
    MIRU_CORE/src/directx12/D3D12DescriptorPoolSet.h
    MIRU_CORE/src/directx12/D3D12Framebuffer.h
    MIRU_CORE/src/directx12/D3D12Image.h
    MIRU_CORE/src/directx12/D3D12Pipeline.h
    MIRU_CORE/src/directx12/D3D12Shader.h
    MIRU_CORE/src/directx12/D3D12Swapchain.h
    MIRU_CORE/src/directx12/D3D12Sync.h
)
set( src_directx12_cpp
    MIRU_CORE/redist/D3D12MemoryAllocator/src/D3D12MemAlloc.cpp
    MIRU_CORE/src/directx12/D3D12Allocator.cpp
    MIRU_CORE/src/directx12/D3D12Buffer.cpp
    MIRU_CORE/src/directx12/D3D12Context.cpp
    MIRU_CORE/src/directx12/D3D12CommandPoolBuffer.cpp
    MIRU_CORE/src/directx12/D3D12DescriptorPoolSet.cpp
    MIRU_CORE/src/directx12/D3D12Framebuffer.cpp
    MIRU_CORE/src/directx12/D3D12Image.cpp
    MIRU_CORE/src/directx12/D3D12Pipeline.cpp
    MIRU_CORE/src/directx12/D3D12Shader.cpp
    MIRU_CORE/src/directx12/D3D12Swapchain.cpp
    MIRU_CORE/src/directx12/D3D12Sync.cpp
)
set( src_vulkan_h
    MIRU_CORE/redist/VulkanMemoryAllocator/src/vk_mem_alloc.h
    MIRU_CORE/src/vulkan/VKAllocator.h
    MIRU_CORE/src/vulkan/VKBuffer.h
    MIRU_CORE/src/vulkan/VKCommandPoolBuffer.h
    MIRU_CORE/src/vulkan/VKContext.h
    MIRU_CORE/src/vulkan/VKDescriptorPoolSet.h
    MIRU_CORE/src/vulkan/VKFramebuffer.h
    MIRU_CORE/src/vulkan/VKImage.h
    MIRU_CORE/src/vulkan/VKPipeline.h
    MIRU_CORE/src/vulkan/VKShader.h
    MIRU_CORE/src/vulkan/VKSwapchain.h
    MIRU_CORE/src/vulkan/VKSync.h
)
set( src_vulkan_cpp
    MIRU_CORE/redist/VulkanMemoryAllocator/src/vk_mem_alloc.cpp
    MIRU_CORE/src/vulkan/VKAllocator.cpp
    MIRU_CORE/src/vulkan/VKBuffer.cpp
    MIRU_CORE/src/vulkan/VKContext.cpp
    MIRU_CORE/src/vulkan/VKCommandPoolBuffer.cpp
    MIRU_CORE/src/vulkan/VKDescriptorPoolSet.cpp
    MIRU_CORE/src/vulkan/VKFramebuffer.cpp
    MIRU_CORE/src/vulkan/VKImage.cpp
    MIRU_CORE/src/vulkan/VKPipeline.cpp
    MIRU_CORE/src/vulkan/VKShader.cpp
    MIRU_CORE/src/vulkan/VKSwapchain.cpp
    MIRU_CORE/src/vulkan/VKSync.cpp
)

set( src_h 
    MIRU_CORE/src/miru_core.h
    MIRU_CORE/src/miru_core_common.h #This is a precompiled header
)
set( src_cpp
    MIRU_CORE/src/miru_core_common.cpp
)

include_directories(
    MIRU_CORE/redist
    MIRU_CORE/src
    ${MIRU_VULKAN_SDK_DIR}/Include
)
link_directories(
    MIRU_CORE/redist
    ${MIRU_VULKAN_SDK_DIR}/Lib
)
add_compile_definitions( SOLUTION_DIR="${CMAKE_CURRENT_SOURCE_DIR}/")

set( src_public
    ${src_crossplatform_h}
    ${src_crossplatform_cpp}
    ${src_debug_h}
    ${src_debug_cpp}
    ${src_directx12_h}
    ${src_directx12_cpp}
    ${src_vulkan_h}
    ${src_vulkan_cpp}
    ${src_h}
    ${src_cpp}
)

set( CMAKE_GENERATOR_PLATFORM Visual Studio 16 2019 Win64)
set( CMAKE_GENERATOR_TOOLSET v142, host=x64)
set( CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY  "${CMAKE_CURRENT_SOURCE_DIR}/MIRU_CORE/lib/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>")

add_library(MIRU_CORE STATIC ${src_public})
add_dependencies(MIRU_CORE MIRU_SHADER_COMPILER)
target_compile_features(MIRU_CORE PUBLIC cxx_std_17)
#target_precompile_headers(MIRU_CORE PUBLIC MIRU_CORE/src/miru_core_common.h )

#MIRU_TEST
set( src_cpp
    MIRU_TEST/src/main.cpp
)

include_directories(
    MIRU_CORE/redist
    MIRU_CORE/src
    $ENV{VULKAN_SDK}/Include
)
link_directories(
    MIRU_CORE/lib/${CMAKE_VS_PLATFORM_NAME}
    MIRU_CORE/redist
    ${MIRU_VULKAN_SDK_DIR}/Lib
)

set( src_public
    ${src_cpp}
)

set( CMAKE_GENERATOR_PLATFORM Visual Studio 16 2019 Win64)
set( CMAKE_GENERATOR_TOOLSET v142, host=x64)
set( CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/MIRU_TEST/exe/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>")

add_executable(MIRU_TEST ${src_public})
add_dependencies(MIRU_TEST MIRU_CORE)
target_link_libraries(MIRU_TEST PUBLIC MIRU_CORE.lib vulkan-1.lib)
target_compile_features(MIRU_TEST PUBLIC cxx_std_17)
set_target_properties( MIRU_TEST PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/MIRU_TEST/")