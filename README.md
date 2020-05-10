# This repository is under active development and is not currently intended for public release or use.

# MIRU
Low level Graphics API wrapper over D3D12 and Vulkan

![MIRU_LOGO](/logo.png)

## Supports:
- API: D3D12 and Vulkan
- Platform: x64 and ARM64
- OS: Windows, Linux and Android
- Graphics Debuggers: Pix, RenderDoc

| OS      | Platform | D3D12            | Vulkan           | Pix for D3D12    | RenderDoc for Vulkan |
| ------- | -------- | ---------------- | ---------------- | ---------------- | -------------------- |
| Windows | x64      |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:    |
| Linux   | x64      |:x:               |:heavy_check_mark:|:x:               |:heavy_check_mark:    |
| Android | ARM64    |:x:               |:heavy_check_mark:|:x:               |:heavy_check_mark:    |

## Projects:
### MIRU_CORE: 
Contains the core functionality of the Low level Graphics API wrapper. Provide path to VulkanSDK; Build as static library; Dynamic Runtime Linking (MD).

### MIRU_SHADER_COMPILER: 
A 'compiler' that uses HLSL files to produce SPIR-V and CSO binaries using glslangValidator and DXC. Provide paths to glslangValidator and DXC; Build as executable; Dynamic Runtime Linking (MD).

### MIRU_TEST: 
Simple test application for development, test and demostration. Build as executable; Dynamic Runtime Linking (MD).

### MIRU_ANDRIOD:
>### MIRU_CORE_ANDRIOD:
>An Android version of the build project version of MIRU_CORE. Provide paths to AndroidSDK and AndroidNDK; Build as static library.

## Build Tools with Visual Studio:
### Windows x64
- Miscrosoft Visual Studio 2019
- Toolset: v142 
- Windows SDK: 10.0.18362
- ISO C++ 17

### Linux x64
- No project available to build.

### Android ARM64
- Miscrosoft Visual Studio 2019
- Toolset: Clang 5.0 
- Android API: 24
- ISO C++ 17 (-std=c++1z)

## Build Tools with CMake:
### Window x64 (Generator Visual Studio 2019 Win64):
- CMake Minimun Versionn 3.17
