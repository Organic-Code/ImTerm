##/////////////////////////////////////
##Kasper de Bruin//////////////////////
##///26-10-23//////////////////////////
##/////////////////////////////////////
#change cmake target to 23
#enabled default project

cmake_minimum_required(VERSION 3.12)
project(ImTerm VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#print values so the user nows why his compiler isnt accepted.
message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID}")

add_library(ImTerm INTERFACE)
add_library(ImTerm::terminal ALIAS ImTerm)

target_include_directories(ImTerm INTERFACE
        $<INSTALL_INTERFACE:include/>
)

add_subdirectory(example EXCLUDE_FROM_ALL)
