cmake_minimum_required(VERSION 3.6.0)

#------VULKAN------
#Sets the Vulkan Include directory for source code build.
set(ANDROID_VULKAN_INCLUDE_DIR ${ANDROID_NDK}/sources/third_party/vulkan/src/include)
include_directories(${ANDROID_VULKAN_INCLUDE_DIR})
#Sets the Vulkan Common directory for load function pointer into the vulkan driver.
set(ANDROID_VULKAN_COMMON_DIR ${ANDROID_NDK}/sources/third_party/vulkan/src/common)
include_directories(${ANDROID_VULKAN_COMMON_DIR})

#---ANDROID_GLUE---
#Sets up the application for native c++ usage.
set(ANDROID_GLUE_INCLUDE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
include_directories(${ANDROID_GLUE_INCLUDE_DIR})
add_library(native_app_glue STATIC ${ANDROID_GLUE_INCLUDE_DIR}/android_native_app_glue.c)
target_include_directories(native_app_glue PUBLIC ${ANDROID_GLUE_INCLUDE_DIR})

#----MIRU_CORE-----
set( MIRU_SOLUTION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../)

set( src_crossplatform_h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/Allocator.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/Buffer.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/CommandPoolBuffer.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/Context.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/DescriptorPoolSet.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/Framebuffer.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/GraphicsAPI.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/Image.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/Pipeline.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/PipelineHelper.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/Shader.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/Swapchain.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/Sync.h
        )
set( src_crossplatform_cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/Allocator.cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/Buffer.cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/CommandPoolBuffer.cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/Context.cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/DescriptorPoolSet.cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/Framebuffer.cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/GraphicsAPI.cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/Image.cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/Pipeline.cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/Shader.cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/Sync.cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/crossplatform/Swapchain.cpp
        )

set( src_debug_h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/debug/GraphicsDebugger.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/debug/Pix.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/debug/RenderDoc.h
        )
set( src_debug_cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/debug/Pix.cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/debug/RenderDoc.cpp
        )

set( src_vulkan_h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/vulkan/VKAllocator.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/vulkan/VKBuffer.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/vulkan/VKCommandPoolBuffer.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/vulkan/VKContext.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/vulkan/VKDescriptorPoolSet.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/vulkan/VKFramebuffer.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/vulkan/VKImage.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/vulkan/VKPipeline.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/vulkan/VKShader.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/vulkan/VKSwapchain.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/vulkan/VKSync.h
        )
set( src_vulkan_cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/vulkan/VKAllocator.cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/vulkan/VKBuffer.cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/vulkan/VKContext.cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/vulkan/VKCommandPoolBuffer.cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/vulkan/VKDescriptorPoolSet.cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/vulkan/VKFramebuffer.cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/vulkan/VKImage.cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/vulkan/VKPipeline.cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/vulkan/VKShader.cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/vulkan/VKSwapchain.cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/vulkan/VKSync.cpp
        )

set( src_h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/miru_core.h
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/miru_core_common.h #This is a precompiled header
        )
set( src_cpp
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src/miru_core_common.cpp
        )

include_directories(
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/redist
)

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

#Sets C++17 and build arguements
#Sets the VK_USE_PLATFORM_ANDROID_KHR and _DEBUG definition
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Werror -D_DEBUG -DSOLUTION_DIR")

add_library(MIRU_CORE STATIC ${src_public})
target_compile_features(MIRU_CORE PUBLIC cxx_std_17)
#target_precompile_headers(MIRU_CORE PUBLIC MIRU_CORE/src/miru_core_common.h )

#---ANDROID_LOG----
#Find Android's Logging Library
find_library(log-lib log)

#-----BUILD_OPT----
#Sets C++17 and build arguements
#Sets the VK_USE_PLATFORM_ANDROID_KHR and _DEBUG definition
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Werror -Wno-exceptions -Wno-unused-private-field -Wno-unused-function -Wno-unused-variable -Wno-missing-braces -D_DEBUG")

#----EXPORT_JNI----
#Set the export of JNIEXPORT void ANativeActivity_onCreate(...);
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

#----MIRU_TEST-----
add_library(MIRU_TEST_ANDROID SHARED
        ${ANDROID_VULKAN_COMMON_DIR}/vulkan_wrapper.cpp #Load PFNs
        src/main/cpp/main.cpp                           #entry point
        )
target_link_libraries(MIRU_TEST_ANDROID
        android
        native_app_glue
        ${log-lib}
        MIRU_CORE
        )