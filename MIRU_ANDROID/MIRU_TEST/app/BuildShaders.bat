cd %~dp0..\..\..\MIRU_SHADER_COMPILER\exe\x64\Debug\
MIRU_SHADER_COMPILER.exe ^
-f:%~dp0..\..\..\MIRU_TEST/res/shaders/basic.hlsl ^
-o:%~dp0..\..\..\MIRU_ANDROID/MIRU_TEST/app/src/main/assets/bin ^
-i:%~dp0..\..\..\MIRU_SHADER_COMPILER/shaders/includes ^
-spv -e:vs_main -s:vert
MIRU_SHADER_COMPILER.exe ^
-f:%~dp0..\..\..\MIRU_TEST/res/shaders/basic.hlsl ^
-o:%~dp0..\..\..\MIRU_ANDROID/MIRU_TEST/app/src/main/assets/bin ^
-i:%~dp0..\..\..\MIRU_SHADER_COMPILER/shaders/includes ^
-spv -e:ps_main -s:frag
pause