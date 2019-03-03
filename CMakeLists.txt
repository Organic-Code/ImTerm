cmake_minimum_required(VERSION 3.12)
project(ImTerm VERSION 0.1.0 LANGUAGES CXX)

add_library(ImTerm INTERFACE)
add_library(ImTerm::terminal ALIAS ImTerm)

target_include_directories(ImTerm INTERFACE
    $<INSTALL_INTERFACE:include/>
)

#add_subdirectory(example EXCLUDE_FROM_ALL)
