cmake_minimum_required(VERSION 3.20)

#MIRU_CORE
set(SRC_CROSSPLATFORM_H
    #src/crossplatform/AccelerationStructure.h
    src/crossplatform/Allocator.h
    src/crossplatform/Buffer.h
    src/crossplatform/CommandPoolBuffer.h
    src/crossplatform/Context.h
    src/crossplatform/DescriptorPoolSet.h
    src/crossplatform/Framebuffer.h
    src/crossplatform/GraphicsAPI.h
    src/crossplatform/Image.h
    src/crossplatform/Pipeline.h
    src/crossplatform/PipelineHelper.h
    src/crossplatform/Shader.h
    src/crossplatform/Swapchain.h
    src/crossplatform/Sync.h
)
set(SRC_CROSSPLATFORM_CPP
    #src/crossplatform/AccelerationStructure.cpp
    src/crossplatform/Allocator.cpp
    src/crossplatform/Buffer.cpp
    src/crossplatform/CommandPoolBuffer.cpp
    src/crossplatform/Context.cpp
    src/crossplatform/DescriptorPoolSet.cpp
    src/crossplatform/Framebuffer.cpp
    src/crossplatform/GraphicsAPI.cpp
    src/crossplatform/Image.cpp
    src/crossplatform/Pipeline.cpp
    src/crossplatform/Shader.cpp
    src/crossplatform/Sync.cpp
    src/crossplatform/Swapchain.cpp
)

set(SRC_DEBUG_H 
    src/debug/GraphicsDebugger.h
    src/debug/Pix.h
    src/debug/RenderDoc.h
)
set(SRC_DEBUG_CPP 
    src/debug/Pix.cpp
    src/debug/RenderDoc.cpp
)

set(SRC_DIRECTX12_H
    redist/D3D12MemoryAllocator/src/D3D12MemAlloc.h
    #src/directx12/D3D12AccelerationStructure.h
    src/directx12/D3D12Allocator.h
    src/directx12/D3D12Buffer.h
    src/directx12/D3D12CommandPoolBuffer.h
    src/directx12/D3D12Context.h
    src/directx12/D3D12DescriptorPoolSet.h
    src/directx12/D3D12Framebuffer.h
    src/directx12/D3D12Image.h
    src/directx12/D3D12Pipeline.h
    src/directx12/D3D12Shader.h
    src/directx12/D3D12Swapchain.h
    src/directx12/D3D12Sync.h
)
set(SRC_DIRECTX12_CPP
    redist/D3D12MemoryAllocator/src/D3D12MemAlloc.cpp
    #src/directx12/D3D12AccelerationStructure.cpp
    src/directx12/D3D12Allocator.cpp
    src/directx12/D3D12Buffer.cpp
    src/directx12/D3D12Context.cpp
    src/directx12/D3D12CommandPoolBuffer.cpp
    src/directx12/D3D12DescriptorPoolSet.cpp
    src/directx12/D3D12Framebuffer.cpp
    src/directx12/D3D12Image.cpp
    src/directx12/D3D12Pipeline.cpp
    src/directx12/D3D12Shader.cpp
    src/directx12/D3D12Swapchain.cpp
    src/directx12/D3D12Sync.cpp
)
set(SRC_VULKAN_H
    redist/VulkanMemoryAllocator/src/vk_mem_alloc.h
    #src/vulkan/VKAccelerationStructure.h
    src/vulkan/VKAllocator.h
    src/vulkan/VKBuffer.h
    src/vulkan/VKCommandPoolBuffer.h
    src/vulkan/VKContext.h
    src/vulkan/VKDescriptorPoolSet.h
    src/vulkan/VKFramebuffer.h
    src/vulkan/VKImage.h
    src/vulkan/VKPipeline.h
    src/vulkan/VKShader.h
    src/vulkan/VKSwapchain.h
    src/vulkan/VKSync.h
)
set(SRC_VULKAN_CPP
    redist/VulkanMemoryAllocator/src/vk_mem_alloc.cpp
    #src/vulkan/VKAccelerationStructure.cpp
    src/vulkan/VKAllocator.cpp
    src/vulkan/VKBuffer.cpp
    src/vulkan/VKContext.cpp
    src/vulkan/VKCommandPoolBuffer.cpp
    src/vulkan/VKDescriptorPoolSet.cpp
    src/vulkan/VKFramebuffer.cpp
    src/vulkan/VKImage.cpp
    src/vulkan/VKPipeline.cpp
    src/vulkan/VKShader.cpp
    src/vulkan/VKSwapchain.cpp
    src/vulkan/VKSync.cpp
)

set(SRC_H 
    src/miru_core.h
    src/miru_core_common.h
)
set(SRC_CPP
    src/miru_core_common.cpp 
)

set(SOURCE_LIST
    ${SRC_CROSSPLATFORM_H}
    ${SRC_CROSSPLATFORM_CPP}
    ${SRC_DEBUG_H}
    ${SRC_DEBUG_CPP}
    ${SRC_DIRECTX12_H}
    ${SRC_DIRECTX12_CPP}
    ${SRC_VULKAN_H}
    ${SRC_VULKAN_CPP}
    ${SRC_H}
    ${SRC_CPP}
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/redist
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${MIRU_VULKAN_SDK_DIR}/Include
)
link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/redist
    ${MIRU_VULKAN_SDK_DIR}/Lib
)

add_compile_definitions(WIN_TARGET_PLATFORM_VERSION=${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
add_compile_definitions(PROJECT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/")

if(MIRU_WIN64_UWP_BUILD)
    add_compile_definitions(MIRU_WIN64_UWP)
endif()

add_library(MIRU_CORE STATIC ${SOURCE_LIST})
add_dependencies(MIRU_CORE MIRU_SHADER_COMPILER)
SetMSVCPrecompiledHeader(MIRU_CORE miru_core_common.h src/miru_core_common.cpp)
set_target_properties(MIRU_CORE PROPERTIES FOLDER ${MIRU_FOLDER})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_LIST})

