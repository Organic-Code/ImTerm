##/////////////////////////////////////
##Kasper de Bruin//////////////////////
##///26-10-23//////////////////////////
##/////////////////////////////////////
#change cmake target to 23

cmake_minimum_required(VERSION 3.12)
project(ImTerm-Example)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/common.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/fmt.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/spdlog.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/SFML.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/imgui-sfml.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/opengl.cmake)

add_executable(ImTerm-Example main.cpp terminal_commands.cpp)
target_include_directories(ImTerm-Example PRIVATE ../include)
target_include_directories(ImTerm-Example SYSTEM PRIVATE ${SFML_INCLUDE_DIR} ${IMGUI_INCLUDE_DIR} ${IMGUI_SFML_INCLUDE_DIR}
        ${SPDLOG_INCLUDE_DIR} ${FMT_INCLUDE_DIR})

target_link_libraries(ImTerm-Example PRIVATE ${SFML_LIBRARY} ${IMGUI_SFML_LIBRARY} ${OPENGL_LIBRARY})

set_target_properties(ImTerm-Example PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
