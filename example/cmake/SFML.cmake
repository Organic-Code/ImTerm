message(STATUS "Configuring SFML")

# Cache config
set(COMPILE_SFML_WITH_PROJECT OFF CACHE BOOL "Compile SFML with project, don't search system installation")

# Config
set(SFML_MINIMUM_SYSTEM_VERSION 2.5)

set(SFML_USE_EMBEDED ${COMPILE_SFML_WITH_PROJECT})
if(NOT SFML_USE_EMBEDED)
	if(CONFIG_OS_WINDOWS)
		set(SFML_USE_EMBEDED ON)
		message(STATUS "OS is Windows, compile SFML with project")
	else()
		find_package(SFML ${SFML_MINIMUM_SYSTEM_VERSION} COMPONENTS system window graphics audio CONFIG)
		if(SFML_FOUND)
			# Variables
			set(SFML_INCLUDE_DIR  "")
			set(SFML_LIBRARY sfml-system sfml-window sfml-graphics sfml-audio)

			# Message
			message(WARNING "If the program crashes when using the clipboard,"
			  " it is an SFML bug not yet fixed on your system but already fixed on Github (see https://github.com/SFML/SFML/pull/1437),"
			  " use CMake with -DCOMPILE_SFML_WITH_PROJECT=ON to compile and use the fixed version with the project")
			message("> include: ${SFML_INCLUDE_DIR}")
			message("> library: ${SFML_LIBRARY}")
			message(STATUS "Configuring SFML - Done")
		else()
			set(SFML_USE_EMBEDED ON)
			message(STATUS "SFML system installation not found, compile SFML with project")
		endif()
	endif()
endif()

if(SFML_USE_EMBEDED)
	get_filename_component(SFML_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/SFML ABSOLUTE)

	# Submodule check
	directory_is_empty(is_empty "${SFML_DIR}")
	if(is_empty)
		message(FATAL_ERROR "SFML dependency is missing, maybe you didn't pull the git submodules")
	endif()

	# Subproject
	add_subdirectory(${SFML_DIR})

	# Configure SFML folder in IDE
	foreach(sfml_target IN ITEMS sfml-system sfml-network sfml-window sfml-graphics sfml-audio sfml-main)
		if(TARGET ${sfml_target})
			set_target_properties(${sfml_target} PROPERTIES FOLDER external/SFML)
		endif()
	endforeach()

	# Configure OpenAL
	if(CONFIG_OS_WINDOWS)
		set(ARCH_FOLDER "x86")
		if(CONFIG_ARCH_64BITS)
			set(ARCH_FOLDER "x64")
		endif()
		configure_file(${SFML_DIR}/extlibs/bin/${ARCH_FOLDER}/openal32.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} COPYONLY)
	endif()

	# Setup targets output, put exe and required SFML dll in the same folder
	target_set_output_directory(sfml-system "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
	target_set_output_directory(sfml-window "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
	target_set_output_directory(sfml-graphics "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
	target_set_output_directory(sfml-audio "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

	# Variables
	get_filename_component(SFML_INCLUDE_DIR  ${SFML_DIR}/include  ABSOLUTE)
	set(SFML_LIBRARY sfml-system sfml-window sfml-graphics sfml-audio)

	# Message
	message("> include: ${SFML_INCLUDE_DIR}")
	message("> library: [compiled with project]")
	message(STATUS "Configuring SFML - Done")
endif()
