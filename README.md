# MIRU
Low level Graphics API wrapper over D3D12 and Vulkan.

![MIRU_LOGO](/Branding/logo.png)

In a broad sense MIRU stands for Multiple Interface Rendering Unit and its name is based on the Japanese verb 'to see' [見る・みる]. This standalone rendering unit is primarily targeted at graphics developers, who work on graphically intensive applications or are developing their own game engine or for an in-house one. The library is intended to be as lean as possible, hence the exclusion of model/texture loading or a crossplatform windowing system; the sole purpose of this library is to draw graphics, execute compute and enable low level access to modern Graphics APIs. The API is based heavily on Vulkan's coding style and is intended to be as clean and modern as possible.
I hope you find this repository useful however you wish to use or learn from it; and any constructive criticism is always welcomed.

This repository is under active development and is not currently intended for commerical release or use.

## Features:
- Crossplatform support with x64 and Android samples.
- Crossplatform implementation of low level primitives such as Buffers, Images, CommandBuffers/Lists, Pipelines etc.
- Built-in Allocator using D3D12MemoryAllocator/VulkanMemoryAllocator for automatic allocation and binding upon resource creation.
- CPU Heap Allocator tracking and CPU Profiler.
- Render Passes with Subpasses for both D3D12 and Vulkan.
- Shader 'compiler' that builds DXIL and SPIR-V shader binaries from .hlsl source.
- Live shader recompilation.
- Shader reflection with dxcompiler for D3D12 and spirv-cross for Vulkan.
- Built-in support for graphics debuggers.
- Assign debug names to low level primitives.
- Ray Tracing support via DXR for D3D12 and via VK_KHR_acceleration_structure/VK_KHR_ray_tracing_pipeline for Vulkan.

## Known issues and features to test/implement:
- Draw/Dispatch commands using indirect buffers.
- D3D12 Root Constants/Vulkan Push constants.
- Vulkan dynamic states and their implementation in D3D12.
- Clear render pass attachments command as an adjunct to load/store operations.
- D3D12 Subpasses conformance.
- Fill/Update Buffer commands.
- Queries for GPU timing.
- Arrays of Descriptors register/binding number conflict.
- Multi GPU support.
- Pipeline Caching?
- Sparse and Aliased resources?
- Other idiosyncratic differences between D3D12 and Vulkan.
- Video Encode and Decode.
- VK_KHR_dynamic_rendering for rendering without RenderPasses and Framebuffers.

## Supports:
- API: D3D12 and Vulkan
- Platform: x64 and ARM64
- OS: Windows, Linux and Android
- Graphics Debuggers: Pix 2108.18, RenderDoc v1.16

| OS      | Platform | D3D12            | Vulkan           | Pix for D3D12    | RenderDoc for D3D12 | RenderDoc for Vulkan |
| ------- | -------- | ---------------- | ---------------- | ---------------- | ------------------- | -------------------- |
| Windows | x64      |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:   |:heavy_check_mark:    |
| Linux   | x64      |:x:               |:heavy_check_mark:|:x:               |:x:                  |:heavy_check_mark:    |
| Android | ARM64    |:x:               |:heavy_check_mark:|:x:               |:x:                  |:heavy_check_mark:    |

## Projects:
### MIRU_CORE: 
Contains the core functionality of the Low level Graphics API wrapper. Provide path to VulkanSDK; Build as dynamic library; Dynamic Runtime Linking (MD).

### MIRU_SHADER_COMPILER: 
A 'compiler' that uses HLSL files to produce SPIR-V and CSO binaries using DXC/FXC. Provide paths to DXC/FXC; Build as executable; Dynamic Runtime Linking (MD).

### MIRU_SHADER_CORE: 
Contains the core functionality for compiling the shaders. Build as static library; Dynamic Runtime Linking (MD).

### MIRU_TEST: 
Simple test application for development, test and demostration. Build as executable; Dynamic Runtime Linking (MD).

### MIRU_TEST_UWP:
A UWP version of the MIRU_TEST project. MIRU_CORE should be built with the define MIRU_WIN64_UWP. Build as executable; Dynamic Runtime Linking (MD). - WIP

### MIRU_ANDRIOD/MIRU_TEST:
An Android version of the MIRU_TEST project. Build both libMIRU_TEST.so, loaded as native activity, and libMIRU_CORE.a;

## Build Tools with Visual Studio:
### Windows x64:
- Microsoft Visual Studio 2022
- Toolset: v143 
- Windows SDK: 10.0.22000.0
- ISO C++ 20
- Microsoft.Windows.CppWinRT NuGet package (For MIRU_TEST_UWP only)

### Linux x64:
- No project available to build.

### Android ARM64:
- Android Studio 4.0.1
- Android SDK Build-Tools: 30.0.2
- Android SDK Platform-Tools: 30.0.1
- Android NDK: 22 Beta / 21.3.6528147
- Android Gradle Plugin Version: 4.0.1
- Gradle Version: 6.1.1
- Android CMake: 3.10.2.4988404
- Android API: Minimum 24 - Target 28
- ISO C++ 17 (-std=c++1z)

