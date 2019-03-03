message(STATUS "Configuring imgui-sfml")

get_filename_component(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui ABSOLUTE)
get_filename_component(IMGUI_SFML_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui-sfml ABSOLUTE)
get_filename_component(IMGUI_SFML_TARGET_DIR ${CMAKE_CURRENT_BINARY_DIR}/imgui-sfml ABSOLUTE)

# Submodules check
directory_is_empty(is_empty "${IMGUI_DIR}")
if(is_empty)
	message(FATAL_ERROR "ImGui dependency is missing, maybe you didn't pull the git submodules")
endif()
directory_is_empty(is_empty "${IMGUI_SFML_DIR}")
if(is_empty)
	message(FATAL_ERROR "imgui-sfml dependency is missing, maybe you didn't pull the git submodules")
endif()

# Configure imgui for imgui-sfml
#if(NOT OPENGL_INCLUDE_DIR OR NOT OPENGL_LIBRARY)
#	message(FATAL_ERROR "Missing OpenGL config")
#endif()
#if(NOT SFML_INCLUDE_DIR OR NOT SFML_LIBRARY)
if(NOT SFML_LIBRARY)
	message(FATAL_ERROR "Missing SFML config")
endif()

# Copy imgui and imgui-sfml files to cmake build folder
configure_folder(${IMGUI_DIR} ${IMGUI_SFML_TARGET_DIR} COPYONLY)
configure_folder(${IMGUI_SFML_DIR} ${IMGUI_SFML_TARGET_DIR} COPYONLY)
# Include imgui-sfml config header in imgui config header
file(APPEND "${IMGUI_SFML_TARGET_DIR}/imconfig.h"
  "\n#include \"imconfig-SFML.h\"\n"
)

# Setup target
get_files(
  files
  "${IMGUI_SFML_TARGET_DIR}"
)
make_target(
  imgui-sfml "generated/external/imgui-sfml" ${files}
  INCLUDES "${OPENGL_INCLUDE_DIR}" "${SFML_INCLUDE_DIR}" "${IMGUI_SFML_TARGET_DIR}"
  OPTIONS cxx no_warnings
)
target_link_libraries(imgui-sfml PRIVATE "${SFML_LIBRARY}" "${OPENGL_LIBRARY}")
target_compile_definitions(imgui-sfml PUBLIC IMGUI_DISABLE_OBSOLETE_FUNCTIONS)

# Variables
get_filename_component(IMGUI_SFML_INCLUDE_DIR  ${IMGUI_SFML_TARGET_DIR}  ABSOLUTE)
set(IMGUI_SFML_LIBRARY imgui-sfml)

# Message
message("> include: ${IMGUI_SFML_INCLUDE_DIR}")
message("> library: [compiled with project]")
message(STATUS "Configuring imgui-sfml - Done")
