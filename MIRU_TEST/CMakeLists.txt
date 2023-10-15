cmake_minimum_required(VERSION 3.27)

#Files
set(SRC_HEADERS
	"src/common.h"
	"src/maths.h"
)
set(SRC_CPP_FILES
	"src/basic.cpp"
	"src/dynamic_rendering.cpp"
	"src/main.cpp"
	"src/mesh_shader.cpp"
	"src/multiview.cpp"
	"src/raytracing.cpp"
	"src/sync2.cpp"
)
set(HLSL_FILES
	"res/shaders/basic.hlsl"
	"res/shaders/meshshader.hlsl"
	"res/shaders/multiview.hlsl"
	"res/shaders/postprocess.hlsl"
	"res/shaders/raytracing.hlsl"
)
set(HLSL_JSON_FILES
	"res/shaders/basic_hlsl.json"
	"res/shaders/meshshader_hlsl.json"
	"res/shaders/multiview_hlsl.json"
	"res/shaders/postprocess_hlsl.json"
	"res/shaders/raytracing_hlsl.json"
)

add_executable(MIRU_TEST)
target_sources(MIRU_TEST PRIVATE ${SRC_CPP_FILES} ${SRC_HEADERS} ${HLSL_FILES} ${HLSL_JSON_FILES})

target_link_libraries(MIRU_TEST PRIVATE MIRU_CORE ARC)

source_group("src" FILES ${SRC_CPP_FILES} ${SRC_HEADERS})
source_group("res\\shaders" FILES ${HLSL_FILES} ${HLSL_JSON_FILES})

set_property(SOURCE ${HLSL_FILES} PROPERTY VS_SETTINGS "ExcludedFromBuild=true")

target_include_directories(MIRU_TEST PRIVATE 
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
	"${CMAKE_CURRENT_SOURCE_DIR}/../MIRU_CORE/src"
	"${CMAKE_CURRENT_SOURCE_DIR}/../External"
)

target_compile_definitions(MIRU_TEST PRIVATE "BUILD_DIR=\"${CMAKE_BINARY_DIR}\"")
target_compile_definitions(MIRU_TEST PRIVATE "SOLUTION_DIR=\"${CMAKE_SOURCE_DIR}\"")

add_dependencies(MIRU_TEST MIRU_SHADER_COMPILER)
add_custom_command(TARGET MIRU_TEST POST_BUILD
	COMMAND MIRU_SHADER_COMPILER -raf:${HLSL_JSON_FILES} 
		-rafd:$BUILD_DIR=\"${CMAKE_BINARY_DIR}\"
		-rafd:$SOLUTION_DIR=\"${CMAKE_SOURCE_DIR}\"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

foreach(HLSL_JSON_FILE ${HLSL_JSON_FILES})
	CopyToBuildDirPostBuild(MIRU_TEST "${CMAKE_CURRENT_SOURCE_DIR}/${HLSL_JSON_FILE}" "shaderbin")
endforeach()