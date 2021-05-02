cmake_minimum_required(VERSION 3.20)

#MIRU_SHADER_COMPILER
set(SRC_H
    src/BuildCSO.h
    src/BuildSPV.h
    src/ErrorCodes.h
    src/MSCDocumentation.h
)
set(SRC_CPP
    src/Main.cpp
)

set(SOURCE_LIST
    ${SRC_H}
    ${SRC_CPP}
)

include_directories(
    ${CMAKE_SOURCE_DIR}/MIRU_CORE/redist
)
link_directories(
    ${CMAKE_SOURCE_DIR}/MIRU_CORE/redist
)

add_compile_definitions(WIN_TARGET_PLATFORM_VERSION=${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})

add_executable(MIRU_SHADER_COMPILER ${SOURCE_LIST})
set_target_properties(MIRU_SHADER_COMPILER PROPERTIES FOLDER ${MIRU_FOLDER})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_LIST})