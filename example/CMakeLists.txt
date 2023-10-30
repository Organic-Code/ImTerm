##/////////////////////////////////////
##Kasper de Bruin//////////////////////
##///26-10-23//////////////////////////
##/////////////////////////////////////
#change cmake target to 23

cmake_minimum_required(VERSION 3.12)
project(ImTerm-Example)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#basic libraries
include(cmake/common.cmake)
include(cmake/fmt.cmake)
include(cmake/spdlog.cmake)
include(cmake/config.cmake)
include(cmake/opengl.cmake)

#Find glfw
find_package(glfw3 3.3 REQUIRED)

# glad sources
set(GLAD_SOURCES external/glad/src/glad.c)

set(IMGUI_SOURCES
        external/imgui/imgui.cpp
        external/imgui/imgui_demo.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_internal.h
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp

        external/imgui/backends/imgui_impl_glfw.cpp
        external/imgui/backends/imgui_impl_opengl3.cpp
)

set(SOURCES
        main.cpp
        terminal_commands.cpp
)



#include imgui
include_directories(external/imgui/)
# include glad
include_directories(external/glad/include)



add_executable(ImTerm-Example ${SOURCES} ${IMGUI_SOURCES} ${GLAD_SOURCES})

target_include_directories(ImTerm-Example PRIVATE ../include)

target_include_directories(
        ImTerm-Example SYSTEM PRIVATE
        ${SPDLOG_INCLUDE_DIR}
        ${FMT_INCLUDE_DIR})

target_link_libraries(
        ImTerm-Example PRIVATE
        glfw)

set_target_properties(
        ImTerm-Example PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF)