message(STATUS "Configuring fmt")

get_filename_component(FMT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/fmt ABSOLUTE)

# Submodule check
directory_is_empty(is_empty "${FMT_DIR}")
if(is_empty)
    message(FATAL_ERROR "Fmt dependency is missing, maybe you didn't pull the git submodules")
endif()

add_compile_definitions(FMT_HEADER_ONLY)
add_subdirectory(${FMT_DIR} EXCLUDE_FROM_ALL)


# Variables
get_filename_component(FMT_INCLUDE_DIR  ${FMT_DIR}/include  ABSOLUTE)
set(FMT_LIBRARY "")

# Message
message("> include: ${FMT_INCLUDE_DIR}")
message(STATUS "Configuring fmt - Done")
