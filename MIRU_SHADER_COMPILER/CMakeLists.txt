cmake_minimum_required(VERSION 3.27)

message(STATUS "MIRU_SHADER_COMPILER")

#Files
set(SRC_HEADERS
	"src/ErrorCodes.h"
	"src/MSCDocumentation.h"
)
set(SRC_CPP_FILES
	"src/Main.cpp"
)
set(SHADER_INCLUDE_HEADERS
	"shaders/includes/msc_common.h"
)

add_executable(MIRU_SHADER_COMPILER)
target_sources(MIRU_SHADER_COMPILER PRIVATE ${SRC_CPP_FILES} ${SRC_HEADERS} ${SHADER_INCLUDE_HEADERS})

target_link_libraries(MIRU_SHADER_COMPILER PRIVATE MIRU_CORE ARC)

source_group("src" FILES ${SRC_CPP_FILES} ${SRC_HEADERS})
source_group("shaders\\include" FILES ${SHADER_INCLUDE_HEADERS})

target_include_directories(MIRU_SHADER_COMPILER PRIVATE 
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
	"${CMAKE_CURRENT_SOURCE_DIR}/../MIRU_CORE/src"
	"${CMAKE_CURRENT_SOURCE_DIR}/../External"
)