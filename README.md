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
- Dynamic Rendering for executing render commands without RenderPasses and Framebuffers (D3D12: Core, Vulkan: VK_KHR_dynamic_rendering).
- Timeline Semaphores for D3D12 style sychronisation primitives (D3D12: Core, Vulkan: VK_KHR_timeline_semaphore).
- Synchronisation 2 for easier management of resource transitions and submissions (D3D12: Enhanced Barriers, Vulkan: VK_KHR_synchronization2).
- ViewInstancing/Multiview supports rendering to multiple views in a single draw call, through the use of SV_ViewID in shaders (D3D12 is limited to a maximum of 4 views).
- Shader Viewport Index Layer allows the usage of SV_RenderTargetArrayIndex and SV_ViewportArrayIndex from pre-rasterisation stages.
- Mesh and Task/Amplification shader support for D3D12 and Vulkan with VK_EXT_mesh_shader.

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

## Supports:
- API: D3D12 and Vulkan
- Platform: x64 and ARM64
- OS: Windows, Linux and Android
- Graphics Debuggers: PIX and RenderDoc

| OS      | Platform | D3D12            | Vulkan           | Pix for D3D12    | RenderDoc for D3D12 | RenderDoc for Vulkan |
| ------- | -------- | ---------------- | ---------------- | ---------------- | ------------------- | -------------------- |
| Windows | x64      |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:   |:heavy_check_mark:    |
| Linux   | x64      |:x:               |:heavy_check_mark:|:x:               |:x:                  |:heavy_check_mark:    |
| Android | ARM64    |:x:               |:heavy_check_mark:|:x:               |:x:                  |:heavy_check_mark:    |

## Projects:
### MIRU_CORE: 
Contains the core functionality of the Low level Graphics API wrapper. Provide path to the Vulkan SDK; Build as dynamic library; Dynamic Runtime Linking (MD).

### MIRU_SHADER_COMPILER: 
A 'compiler' that uses HLSL files to produce SPIR-V and CSO binaries using dxcompiler. Build as executable; Dynamic Runtime Linking (MD).

### MIRU_TEST: 
Simple test application for development, test and demostration. Build as executable; Dynamic Runtime Linking (MD). This project also compiles for Android with Android Game Developer Extension.

### MIRU_TEST_UWP:
A UWP version of the MIRU_TEST project. MIRU_CORE should be built with the define MIRU_WIN64_UWP. Build as executable; Dynamic Runtime Linking (MD). - WIP

## CMake and Visual Studio:
### Windows x64:
- Microsoft Visual Studio 2022
- Toolset: v143 
- Windows SDK: 10.0.22621.0
- ISO C++ 20
- Vulkan SDK 1.3 with Shader Toolchain Debug Symbols - 64-bit
- NuGet Package: Microsoft.Direct3D.D3D12 1.614.1
- NuGet Package: Microsoft.Direct3D.DXC 1.8.2407.12
- NuGet Package: WinPixEventRuntime 1.0.240308001
