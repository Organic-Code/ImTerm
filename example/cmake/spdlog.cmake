message(STATUS "Configuring spdlog")

get_filename_component(SPDLOG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/spdlog ABSOLUTE)

# Submodule check
directory_is_empty(is_empty "${SPDLOG_DIR}")
if(is_empty)
	message(FATAL_ERROR "Spdlog dependency is missing, maybe you didn't pull the git submodules")
endif()

# Variables
get_filename_component(SPDLOG_INCLUDE_DIR  ${SPDLOG_DIR}/include  ABSOLUTE)
set(SPDLOG_LIBRARY "")

# Message
message("> include: ${SPDLOG_INCLUDE_DIR}")
message(STATUS "Configuring spdlog - Done")
