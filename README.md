# This repository is under active development and is not currently intended for public release or use.

# MIRU
Low level Graphics API wrapper over D3D12 and Vulkan

![MIRU_LOGO](/logo.png)

## Supports:
- API: D3D12 and Vulkan
- Platform: x64 only
- OS: Windows, Linux (Vulkan only)
- Graphics Debugger: RenderDoc

## Projects:
MIRU_CORE: Contains the core functionality of the Low level Graphics API wrapper. Provide paths to VulkanSDK; Build as static library; Dynamic Runtime Linking (MD).

MIRU_SHADER_COMPILER: A compiler designed to create GLSL and HLSL files for glslangValidator and DXC to produce SPIR-V and CSO binaries from the proprietary shading language (MSL). Provide paths to glslangValidator and DXC; Build as executable; Dynamic Runtime Linking (MD).

MIRU_TEST: Simple test application for development, test and demostration. Build as executable; Dynamic Runtime Linking (MD).

## Build Tools:
- Miscrosoft Visual Studio 2019
- Toolset: v142 
- Windows SDK: 10.0.18362
