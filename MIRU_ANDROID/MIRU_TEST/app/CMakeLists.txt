cmake_minimum_required(VERSION 3.6.0)

#MIRU_SOLUTION_DIR
set( MIRU_SOLUTION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../)

#------VULKAN------
#Sets the Vulkan Include directory for source code build.
set(ANDROID_VULKAN_INCLUDE_DIR ${ANDROID_NDK}/sources/third_party/vulkan/src/include)
include_directories(${ANDROID_VULKAN_INCLUDE_DIR})

#---ANDROID_GLUE---
#Sets up the application for native c++ usage.
set(ANDROID_GLUE_INCLUDE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
include_directories(${ANDROID_GLUE_INCLUDE_DIR})
add_library(native_app_glue STATIC ${ANDROID_GLUE_INCLUDE_DIR}/android_native_app_glue.c)
target_include_directories(native_app_glue PUBLIC ${ANDROID_GLUE_INCLUDE_DIR})

#-SPIRV_CROSS_CORE-
set(SPIRV_CROSS_CORE_DIR D:/MASS\ STORAGE/SPIRV-Cross)
set(BUILD_SPIRV_CROSS_CORE FALSE)
if(BUILD_SPIRV_CROSS_CORE)
    set(spirv_cross_core_src
            ${SPIRV_CROSS_CORE_DIR}/GLSL.std.450.h
            ${SPIRV_CROSS_CORE_DIR}/spirv_common.hpp
            ${SPIRV_CROSS_CORE_DIR}/spirv_cross_containers.hpp
            ${SPIRV_CROSS_CORE_DIR}/spirv_cross_error_handling.hpp
            ${SPIRV_CROSS_CORE_DIR}/spirv.hpp
            ${SPIRV_CROSS_CORE_DIR}/spirv_cross.hpp
            ${SPIRV_CROSS_CORE_DIR}/spirv_cross.cpp
            ${SPIRV_CROSS_CORE_DIR}/spirv_parser.hpp
            ${SPIRV_CROSS_CORE_DIR}/spirv_parser.cpp
            ${SPIRV_CROSS_CORE_DIR}/spirv_cross_parsed_ir.hpp
            ${SPIRV_CROSS_CORE_DIR}/spirv_cross_parsed_ir.cpp
            ${SPIRV_CROSS_CORE_DIR}/spirv_cfg.hpp
            ${SPIRV_CROSS_CORE_DIR}/spirv_cfg.cpp
            )

    add_library(spirv-cross-core STATIC ${spirv_cross_core_src})
    target_include_directories(spirv-cross-core PUBLIC ${SPIRV_CROSS_CORE_DIR})

    if(FALSE)
    #Copy to redist folder
    file(COPY ${CMAKE_CURRENT_BINARY_DIR}/libspirv-cross-core.a
        DESTINATION ${MIRU_SOLUTION_DIR}/MIRU_CORE/redist/spirv_cross/lib/ARM64)
    endif()
endif()

#----MIRU_CORE-----
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
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/redist/VulkanMemoryAllocator/src/vk_mem_alloc.h
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
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/redist/VulkanMemoryAllocator/src/vk_mem_alloc.cpp
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
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/dep
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/src
        ${MIRU_SOLUTION_DIR}/MIRU_CORE/redist
)

set( src_public
        ${src_crossplatform_h}
        ${src_crossplatform_cpp}
        ${src_debug_h}
        ${src_debug_cpp}
        ${src_vulkan_h}
        ${src_vulkan_cpp}
        ${src_h}
        ${src_cpp}
        )

#Sets C++17 and build arguements
#Sets the VK_USE_PLATFORM_ANDROID_KHR and _DEBUG definition
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Werror -Wno-unused-variable -DVK_USE_PLATFORM_ANDROID_KHR -D_DEBUG -DSOLUTION_DIR")

add_library(MIRU_CORE STATIC ${src_public})
link_directories(${MIRU_SOLUTION_DIR}/MIRU_CORE/redist/spirv_cross/lib/ARM64)
target_compile_features(MIRU_CORE PUBLIC cxx_std_17)
target_link_libraries(MIRU_CORE spirv-cross-core)
#target_precompile_headers(MIRU_CORE PUBLIC MIRU_CORE/src/miru_core_common.h )

#---ANDROID_LOG----
#Find Android's Logging Library
find_library(log-lib log)

#-----BUILD_OPT----
#Sets C++17 and build arguements
#Sets the VK_USE_PLATFORM_ANDROID_KHR and _DEBUG definition
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Werror -Wno-unused-variable -Wno-self-assign-field -DVK_USE_PLATFORM_ANDROID_KHR -D_DEBUG")

#----EXPORT_JNI----
#Set the export of JNIEXPORT void ANativeActivity_onCreate(...);
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

#----MIRU_TEST-----
include_directories(${MIRU_SOLUTION_DIR}/MIRU_ANDROID/MIRU_TEST/app/src/main/cpp ${MIRU_TEST_MARS_DIR})
add_library(MIRU_TEST SHARED
        src/main/cpp/vulkan_wrapper.h           #Load PFNs
        src/main/cpp/vulkan_wrapper.cpp         #Load PFNs
        src/main/cpp/main.cpp                   #entry point
        )
target_link_libraries(MIRU_TEST
        android
        native_app_glue
        ${log-lib}
        MIRU_CORE
        )