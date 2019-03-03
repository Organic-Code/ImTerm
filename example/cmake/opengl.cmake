message(STATUS "Configuring OpenGL")

# Find OpenGL
set(OpenGL_GL_PREFERENCE "LEGACY")
find_package(OpenGL REQUIRED)

if(NOT OPENGL_FOUND)
	message(FATAL_ERROR "OpenGL not found")
	return()
endif()

# Variables
set(OPENGL_INCLUDE_DIR   ${OPENGL_INCLUDE_DIRS})
set(OPENGL_LIBRARY       ${OPENGL_gl_LIBRARY})

# Message
message(STATUS "Configuring OpenGL - Done")
