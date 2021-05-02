cmake_minimum_required(VERSION 3.20)

#MIRU_TEST
set(SRC_CPP
    src/main.cpp
)

set(SOURCE_LIST
    ${SRC_CPP}
)

include_directories(
    ${CMAKE_SOURCE_DIR}/MIRU_CORE/redist
    ${CMAKE_SOURCE_DIR}/MIRU_CORE/src
    ${MIRU_VULKAN_SDK_DIR}/Include
)
link_directories(
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    ${CMAKE_SOURCE_DIR}/MIRU_CORE/redist
    ${MIRU_VULKAN_SDK_DIR}/Lib
)

LoadExternalFolder(RES_LIST res)

add_executable(MIRU_TEST ${SOURCE_LIST} ${RES_LIST})
add_dependencies(MIRU_TEST MIRU_CORE)
target_link_libraries(MIRU_TEST PUBLIC MIRU_CORE.lib vulkan-1.lib)
set_target_properties(MIRU_TEST PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(MIRU_TEST PROPERTIES FOLDER ${MIRU_FOLDER})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_LIST})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${RES_LIST})