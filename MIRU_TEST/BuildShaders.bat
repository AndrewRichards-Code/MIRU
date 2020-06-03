cd %~dp0..\MIRU_SHADER_COMPILER\exe\x64\Debug\
MIRU_SHADER_COMPILER.exe ^
-f:%~dp0res/shaders/basic.vert.hlsl ^
-o:%~dp0res/bin ^
-i:%~dp0../MIRU_SHADER_COMPILER/shaders/includes ^
-cso -spv -t:6_4 
MIRU_SHADER_COMPILER.exe ^
-f:%~dp0res/shaders/basic.frag.hlsl ^
-o:%~dp0res/bin ^
-i:%~dp0../MIRU_SHADER_COMPILER/shaders/includes ^
-cso -spv -t:6_4 
pause