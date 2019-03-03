##################################################################################
# MIT License                                                                    #
#                                                                                #
# Copyright (c) 2017 Maxime Pinard                                               #
#                                                                                #
# Permission is hereby granted, free of charge, to any person obtaining a copy   #
# of this software and associated documentation files (the "Software"), to deal  #
# in the Software without restriction, including without limitation the rights   #
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      #
# copies of the Software, and to permit persons to whom the Software is          #
# furnished to do so, subject to the following conditions:                       #
#                                                                                #
# The above copyright notice and this permission notice shall be included in all #
# copies or substantial portions of the Software.                                #
#                                                                                #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     #
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       #
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    #
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         #
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  #
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  #
# SOFTWARE.                                                                      #
##################################################################################

include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)

## check_variable_name(function_name var_names... USED used_var_names...)
# Check if variables passed to a function don't have names conflicts with variables used in the function.
# Generate a fatal error on variable name conflict.
#   {value} [in] function_name:    Function name
#   {value} [in] var_names:        Names of the variables passed as parameters of the function
#   {value} [in] used_var_names:   Names of the function internal variables
function(check_variable_name _function_name)
	split_args(_var_names USED _used_var_names ${ARGN})
	foreach(_var_name ${_var_names})
		foreach(_used_var_name ${_used_var_names})
			if("${_var_name}" STREQUAL "${_used_var_name}")
				message(FATAL_ERROR "parameter ${_var_name} passed to function ${_function_name} has the same name than an internal variable of the function and can't be accessed")
			endif()
		endforeach()
	endforeach()
endfunction()

## directory_is_empty(output dir)
# Check if a directory is empty.
# If the input directory doesn't exist or is not a directory, it is considered as empty.
#   {variable} [out] output:   true if the directory is empty, false otherwise
#   {value}    [in]  dir:      Directory to check
function(directory_is_empty output dir)
	set(tmp_output false)
	get_filename_component(dir_path ${dir} REALPATH)
	if(EXISTS "${dir_path}")
		if(IS_DIRECTORY "${dir_path}")
			file(GLOB files "${dir_path}/*")
			list(LENGTH files len)
			if(len EQUAL 0)
				set(tmp_output true)
			endif()
		else()
			set(tmp_output true)
		endif()
	else()
		set(tmp_output true)
	endif()
	set(${output} ${tmp_output} PARENT_SCOPE)
endfunction()

## split_args(left delimiter right args...)
# Split arguments into left and right parts on delimiter token.
#   {variable} [out] left:        Arguments at the left (before) the delimiter
#   {value}    [in]  delimiter:   Delimiter of the left and right part
#   {variable} [out] right:       Arguments at the right (after) the delimiter
#   {value}    [in]  args:        Arguments to split
function(split_args left delimiter right)
	set(delimiter_found false)
	set(tmp_left)
	set(tmp_right)
	foreach(it ${ARGN})
		if("${it}" STREQUAL ${delimiter})
			set(delimiter_found true)
		elseif(delimiter_found)
			list(APPEND tmp_right ${it})
		else()
			list(APPEND tmp_left ${it})
		endif()
	endforeach()
	set(${left} ${tmp_left} PARENT_SCOPE)
	set(${right} ${tmp_right} PARENT_SCOPE)
endfunction()

## has_item(output item args...)
# Determine if arguments list contains an item.
#   {variable} [out] output:   true if the list contains the item, false otherwise
#   {value}    [in]  item:     Item to search in the list
#   {value}    [in]  args:     Arguments list which may contain the item
function(has_item output item)
	set(tmp_output false)
	foreach(it ${ARGN})
		if("${it}" STREQUAL "${item}")
			set(tmp_output true)
			break()
		endif()
	endforeach()
	set(${output} ${tmp_output} PARENT_SCOPE)
endfunction()

## flatten(property)
# Remove leading, trailing and multiple from a property.
# Useful to prevent unnecessary rebuilds.
#   {variable} [in,out] property:   Property to flatten
function(flatten _property)
	# variables names are prefixed with'_' lower chances of parent scope variable hiding
	check_variable_name(flatten ${_property} USED _property _tmp_property)
	string(REPLACE "  " " " _tmp_property "${${_property}}")
	string(STRIP "${_tmp_property}" _tmp_property)
	set(${_property} "${_tmp_property}" PARENT_SCOPE)
endfunction()

## manage_flag(property regexp flag)
# Add a flag to a property or replace an existing flag matching a regular expression.
# If a flag of the property matches the regular expression, it is replaced by the input flag,
# else, the input flag is added to the property.
# Update the property in the cache.
#   {variable} [in] property:   Property to manage
#   {value}    [in] regexp:     Regular expression to match
#   {value}    [in] flag:       Flag to add (or use to replace regexp)
function(manage_flag _property _regexp _flag)
	# variables names are prefixed with'_' lower chances of parent scope variable hiding
	check_variable_name(manage_flag ${_property} USED _property _regexp _flag)
	if("${${_property}}" MATCHES ${_regexp})
		string(REGEX REPLACE ${_regexp} ${_flag} ${_property} "${${_property}}")
	else()
		set(${_property} "${${_property}} ${_flag}")
	endif()
	flatten(${_property})
	set(${_property} "${${_property}}" CACHE STRING "" FORCE)
endfunction()

## add_flag(property flag [configs...])
# If not already present, add a flag to a property for the specified configs.
#   {value} [in] property:   Property to change (<LANG>|(EXE|MODULE|SHARED|STATIC)_LINKER)
#   {value} [in] value:      Flag to add
#   {value} [in] configs:    Configs for the property to change (DEBUG|RELEASE|RELWITHDEBINFO)
function(add_flag property flag)
	if(ARGN)
		foreach(config ${ARGN})
			manage_flag(CMAKE_${property}_FLAGS_${config} ${flag} ${flag})
		endforeach()
	else()
		manage_flag(CMAKE_${property}_FLAGS ${flag} ${flag})
	endif()
endfunction()

## remove_flag(property regexp [configs...])
# Remove an existing flag of a property matching a regular expression for the specified configs.
#   {value} [in] property:   Property to change (<LANG>|(EXE|MODULE|SHARED|STATIC)_LINKER)
#   {value} [in] regexp:     Regular expression to match
#   {value} [in] configs:    Configs for the property to change (DEBUG|RELEASE|RELWITHDEBINFO)
function(remove_flag property regexp)
	if(ARGN)
		foreach(config ${ARGN})
			manage_flag(CMAKE_${property}_FLAGS_${config} ${regexp} " ")
		endforeach()
	else()
		manage_flag(CMAKE_${property}_FLAGS ${regexp} " ")
	endif()
endfunction()

## add_linker_flag(flag [configs...])
# Add a flag to the linker arguments property for the specified configs.
#   {value} [in] flag:      Flag to add
#   {value} [in] configs:   Configs for the property to change (DEBUG|RELEASE|RELWITHDEBINFO)
function(add_linker_flag flag)
	foreach(item IN ITEMS EXE MODULE SHARED STATIC)
		add_flag(${item}_LINKER ${flag} ${ARGN})
	endforeach()
endfunction()

## remove_linker_flag(flag [configs...])
# Remove a flag from the linker arguments property for the specified configs.
#   {value} [in] flag:      Flag to remove
#   {value} [in] configs:   Configs for the property to change (DEBUG|RELEASE|RELWITHDEBINFO)
function(remove_linker_flag regexp)
	foreach(item IN ITEMS EXE MODULE SHARED STATIC)
		remove_flag(${item}_LINKER ${regexp} ${ARGN})
	endforeach()
endfunction()

## add_cx_flag(flag [configs...])
# Add a flag to the C and CXX compilers arguments property for the specified configs.
#   {value} [in] flag:      Flag to add
#   {value} [in] configs:   Configs for the property to change (DEBUG|RELEASE|RELWITHDEBINFO)
function(add_cx_flag flag)
	CHECK_CXX_COMPILER_FLAG(${flag} has${flag})
	if(has${flag})
		add_flag(C ${flag} ${ARGN})
		add_flag(CXX ${flag} ${ARGN})
	endif()
endfunction()

## remove_cx_flag(flag [configs...])
# Remove a flag from the C and CXX compilers arguments property for the specified configs.
#   {value} [in] flag:      Flag to remove
#   {value} [in] configs:   Configs for the property to change (DEBUG|RELEASE|RELWITHDEBINFO)
function(remove_cx_flag regexp)
	remove_flag(C ${regexp} ${ARGN})
	remove_flag(CXX ${regexp} ${ARGN})
endfunction()

## group_files(group root files...)
# Group files in IDE project generation (by calling source_group) relatively to a root.
#   {value} [in] group:   Group, files will be grouped in
#   {value} [in] root:    Root, files will be grouped relative to it
#   {value} [in] files:   Files to group
function(group_files group root)
	foreach(it ${ARGN})
		get_filename_component(dir ${it} PATH)
		file(RELATIVE_PATH relative ${root} ${dir})
		set(local ${group})
		if(NOT "${relative}" STREQUAL "")
			set(local "${group}/${relative}")
		endif()
		# replace '/' and '\' (and repetitions) by '\\'
		string(REGEX REPLACE "[\\\\\\/]+" "\\\\\\\\" local ${local})
		source_group("${local}" FILES ${it})
	endforeach()
endfunction()

## filter_list(keep list regexp...)
# Include/exclude items from a list matching one or more regular expressions.
# Include: keep only the list items that matches regular expressions.
# Exclude: keep only the list items that doesn't matches regular expressions.
#   {value}    [in]     keep:     1 for include, 0 for exclude
#   {variable} [in,out] list:     List to filter
#   {value}    [in]     regexp:   Regular expressions which list items may match
function(filter_list _keep _list)
	# variables names are prefixed with'_' lower chances of parent scope variable hiding
	check_variable_name(filter_list ${_list} USED _list _tmp_list)
	set(_tmp_list)
	foreach(_it ${${_list}})
		set(_touch)
		foreach(_regexp ${ARGN})
			if(${_it} MATCHES ${_regexp})
				set(_touch true)
				break()
			endif()
		endforeach()
		if((_keep EQUAL 1 AND DEFINED _touch) OR (_keep EQUAL 0 AND NOT DEFINED _touch))
			list(APPEND _tmp_list ${_it})
		endif()
	endforeach()
	set(${_list} ${_tmp_list} PARENT_SCOPE)
endfunction()

## filter_out(list regexp...)
# Exclude items from a list matching one or more regular expressions.
# Exclude: keep only the list items that doesn't matches regular expressions.
#   {variable} [in,out] list:     List to filter
#   {value}    [in]     regexp:   Regular expressions which list items may match
macro(filter_out list)
	filter_list(0 ${list} ${ARGN})
endmacro()

## filter_in(list regexp...)
# Include items from a list matching one or more regular expressions.
# Include: keep only the list items that matches regular expressions.
#   {variable} [in,out] list:     List to filter
#   {value}    [in]     regexp:   Regular expressions which list items may match
macro(filter_in list)
	filter_list(1 ${list} ${ARGN})
endmacro()

## join_list(output sep items...)
# Join items with a separator into a variable.
#   {variable} [out] output:   Output variable, contain the item joined
#   {value}    [in]  sep:      Separator to insert between items
#   {value}    [in]  items:    Items to join
function(join_list output sep)
	set(tmp_output)
	set(first)
	foreach(it ${ARGN})
		if(NOT DEFINED first)
			set(tmp_output ${it})
			set(first true)
		else()
			set(tmp_output "${tmp_output}${sep}${it}")
		endif()
	endforeach()
	set(${output} ${tmp_output} PARENT_SCOPE)
endfunction()

## get_files(output_files directories... [OPTIONS [recurse]])
# Get (recursively or not) C and C++ sources files form input directories.
# Also group the files into Sources with the group_files function relatively to their root directory.
#   {variable} [out] output:        Output variable, contain the sources files
#   {value}    [in]  directories:   Directory to search files
#   {option}   [in]  recurse:       If present, search is recursive
function(get_files output)
	split_args(dirs "OPTIONS" options ${ARGN})
	set(glob GLOB)
	has_item(has_recurse "recurse" ${options})
	if(has_recurse)
		set(glob GLOB_RECURSE)
	endif()
	set(files)
	foreach(it ${dirs})
		if(IS_DIRECTORY ${it})
			set(patterns
			  "${it}/*.c"
			  "${it}/*.cc"
			  "${it}/*.cpp"
			  "${it}/*.cxx"
			  "${it}/*.h"
			  "${it}/*.hpp"
			  )
			file(${glob} tmp_files ${patterns})
			list(APPEND files ${tmp_files})
			get_filename_component(parent_dir ${it} DIRECTORY)
			group_files(Sources "${parent_dir}" ${tmp_files})
		else()
			list(APPEND files ${it})
			get_filename_component(dir ${it} DIRECTORY)
			group_files(Sources "${dir}" ${it})
		endif()
	endforeach()
	set(${output} ${files} PARENT_SCOPE)
endfunction()

## target_add_includes(target includes...)
# Add input includes (files or directories) to target.
#   {value} [in] target:     Target to add includes
#   {value} [in] includes:   Includes to add
function(target_add_includes target)
	list(LENGTH ARGN size)
	if(NOT ${size} GREATER 0)
		return()
	endif()
	list(REMOVE_DUPLICATES ARGN)
	list(SORT ARGN)
	target_include_directories(${target} PRIVATE ${ARGN})
endfunction()

## target_add_system_includes(target includes...)
# Add input system includes (files or directories) to target.
#   {value} [in] target:     Target to add includes
#   {value} [in] includes:   Includes to add
function(target_add_system_includes target)
	list(LENGTH ARGN size)
	if(NOT ${size} GREATER 0)
		return()
	endif()
	list(REMOVE_DUPLICATES ARGN)
	list(SORT ARGN)
	target_include_directories(${target} SYSTEM PRIVATE ${ARGN})
endfunction()

## target_add_compile_definition(target definition [configs...])
# Add a private compile definition to the target for the specified configs.
#   {value} [in] target:       Target to add flag
#   {value} [in] definition:   Definition to add
#   {value} [in] configs:      Configs for the property to change (DEBUG|RELEASE|RELWITHDEBINFO)
function(target_add_compile_definition target definition)
	if(${ARGC} GREATER 2)
		foreach(config ${ARGN})
			string(TOLOWER "${config}" config_lower)
			set(config_name)
			foreach(valid_config IN ITEMS "Debug" "RelWithDebInfo" "Release")
				string(TOLOWER "${valid_config}" valid_config_lower)
				if(${config_lower} STREQUAL ${valid_config_lower})
					set(config_name ${valid_config})
				endif()
			endforeach()
			if(DEFINED config_name)
				target_compile_definitions(${target} PRIVATE "$<$<CONFIG:${config_name}>:${definition}>")
			endif()
		endforeach()
	else()
		target_compile_definitions(${target} PRIVATE "${definition}")
	endif()
endfunction()

## target_add_compiler_flag(target flag [configs...])
# Add a flag to the compiler arguments of the target for the specified configs.
# Add the flag only if the compiler support it (checked with CHECK_CXX_COMPILER_FLAG).
#   {value} [in] target:    Target to add flag
#   {value} [in] flag:      Flag to add
#   {value} [in] configs:   Configs for the property to change (DEBUG|RELEASE|RELWITHDEBINFO)
function(target_add_compiler_flag target flag)
	CHECK_CXX_COMPILER_FLAG(${flag} has${flag})
	if(has${flag})
		if(${ARGC} GREATER 2)
			foreach(config ${ARGN})
				string(TOLOWER "${config}" config_lower)
				set(config_name)
				foreach(valid_config IN ITEMS "Debug" "RelWithDebInfo" "Release")
					string(TOLOWER "${valid_config}" valid_config_lower)
					if(${config_lower} STREQUAL ${valid_config_lower})
						set(config_name ${valid_config})
					endif()
				endforeach()
				if(DEFINED config_name)
					target_compile_options(${target} PRIVATE "$<$<CONFIG:${config_name}>:${flag}>")
				endif()
			endforeach()
		else()
			target_compile_options(${target} PRIVATE "${flag}")
		endif()
	endif()
endfunction()

## append_to_target_property(target property [values...])
# Append values to a target property.
#   {value} [in] target:     Target to modify
#   {value} [in] property:   Property to append values to
#   {value} [in] values:     Values to append to the property
function(append_to_target_property target property)
	set(new_values ${ARGN})
	get_target_property(existing_values ${target} ${property})
	if(existing_values)
		set(new_values "${existing_values} ${new_values}")
	endif()
	set_target_properties(${target} PROPERTIES ${property} ${new_values})
endfunction()

## __target_link_flag_property(target flag  [configs...])
# Add a flag to the linker arguments of the target for the specified configs using LINK_FLAGS properties of the target.
# Function made for CMake 3.12 or less, future CMake version will have target_link_options() with cmake-generator-expressions.
#   {value} [in] target:    Target to add flag
#   {value} [in] flag:      Flag to add
#   {value} [in] configs:   Configs for the property to change (DEBUG|RELEASE|RELWITHDEBINFO)
function(__target_link_flag_property target flag)
	if(${ARGC} GREATER 2)
		foreach(config ${ARGN})
			append_to_target_property(${target} LINK_FLAGS_${config} ${flag})
		endforeach()
	else()
		append_to_target_property(${target} LINK_FLAGS ${flag})
	endif()
endfunction()

## target_add_linker_flag(target flag [configs...])
# Add a flag to the linker arguments of the target for the specified configs.
# Add the flag only if the linker support it (checked with CHECK_CXX_COMPILER_FLAG).
#   {value} [in] target:    Target to add flag
#   {value} [in] flag:      Flag to add
#   {value} [in] configs:   Configs for the property to change (DEBUG|RELEASE|RELWITHDEBINFO)
function(target_add_linker_flag target flag)
	CHECK_CXX_COMPILER_FLAG(${flag} has${flag})
	if(has${flag})
		if(${ARGC} GREATER 2)
			foreach(config ${ARGN})
				string(TOLOWER "${config}" config_lower)
				set(config_name)
				foreach(valid_config IN ITEMS "Debug" "RelWithDebInfo" "Release")
					string(TOLOWER "${valid_config}" valid_config_lower)
					if(${config_lower} STREQUAL ${valid_config_lower})
						set(config_name ${valid_config})
					endif()
				endforeach()
				if(DEFINED config_name)
					if(COMMAND target_link_options)
						target_link_options(${target} PRIVATE "$<$<CONFIG:${config_name}>:${flag}>")
					else()
						string(TOUPPER "${config_name}" config_name_upper)
						__target_link_flag_property(${target} ${flag} ${config_name_upper})
					endif()
				endif()
			endforeach()
		else()
			if(COMMAND target_link_options)
				target_link_options(${target} PRIVATE "${flag}")
			else()
				__target_link_flag_property(${target} "${flag}")
			endif()
		endif()
	endif()
endfunction()

## target_set_output_directory(target directory)
# Set the target runtime, library and archive output directory to the input directory.
#   {value} [in] target:      Target to configure
#   {value} [in] directory:   Output directory
function(target_set_output_directory target directory)
	target_set_runtime_output_directory(${target} "${directory}")
	target_set_library_output_directory(${target} "${directory}")
	target_set_archive_output_directory(${target} "${directory}")
endfunction()

## target_set_runtime_output_directory(target directory)
# Set the target runtime output directory to the input directory.
#   {value} [in] target:      Target to configure
#   {value} [in] directory:   Output directory
function(target_set_runtime_output_directory target directory)
	set_target_properties(${target} PROPERTIES
	  RUNTIME_OUTPUT_DIRECTORY                "${directory}"
	  RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${directory}"
	  RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${directory}"
	  RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${directory}"
	  )
endfunction()

## target_set_library_output_directory(target directory)
# Set the target library output directory to the input directory.
#   {value} [in] target:      Target to configure
#   {value} [in] directory:   Output directory
function(target_set_library_output_directory target directory)
	set_target_properties(${target} PROPERTIES
	  LIBRARY_OUTPUT_DIRECTORY                "${directory}"
	  LIBRARY_OUTPUT_DIRECTORY_DEBUG          "${directory}"
	  LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${directory}"
	  LIBRARY_OUTPUT_DIRECTORY_RELEASE        "${directory}"
	  )
endfunction()

## target_set_archive_output_directory(target directory)
# Set the target archive output directory to the input directory.
#   {value} [in] target:      Target to configure
#   {value} [in] directory:   Output directory
function(target_set_archive_output_directory target directory)
	set_target_properties(${target} PROPERTIES
	  ARCHIVE_OUTPUT_DIRECTORY                "${directory}"
	  ARCHIVE_OUTPUT_DIRECTORY_DEBUG          "${directory}"
	  ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${directory}"
	  ARCHIVE_OUTPUT_DIRECTORY_RELEASE        "${directory}"
	  )
endfunction()

## setup_msvc(target [OPTIONS [static_runtime] [no_warnings] [low_warnings]])
# Set up msvc-specific options of the target.
# Without options: maximum warnings are enabled.
#   {value}  [in] target:           Target to configure
#   {option} [in] static_runtime:   If present, C runtime library is statically linked
#   {option} [in] no_warnings:      If present, warnings are disabled (useful for external projects)
#   {option} [in] low_warnings:     If present, low/normal warnings are enabled
function(setup_msvc target)
	split_args(ignore "OPTIONS" options ${ARGN})
	has_item(option_static_runtime "static_runtime" ${options})
	has_item(option_no_warnings "no_warnings" ${options})
	has_item(option_low_warnings "low_warnings" ${options})

	# set Source and Executable character sets to UTF-8
	target_add_compiler_flag(${target} "/utf-8")

	# enable parallel compilation
	target_add_compiler_flag(${target} "/MP")

	# generates complete debugging information
	target_add_compiler_flag(${target} "/Zi" DEBUG RELWITHDEBINFO)
	target_add_linker_flag(${target} "/DEBUG:FULL" DEBUG RELWITHDEBINFO)

	# set optimization
	target_add_compiler_flag(${target} "/Od" DEBUG)
	target_add_compiler_flag(${target} "/O2" RELWITHDEBINFO)
	target_add_compiler_flag(${target} "/Ox" RELEASE)

	# enables automatic parallelization of loops
	target_add_compiler_flag(${target} "/Qpar" RELEASE)

	# enable runtime checks
	target_add_compiler_flag(${target} "/RTC1" DEBUG)

	# disable incremental compilations
	target_add_linker_flag(${target} "/INCREMENTAL:NO" RELEASE RELWITHDEBINFO)

	# remove unused symbols
	target_add_linker_flag(${target} "/OPT:REF" RELEASE RELWITHDEBINFO)
	target_add_linker_flag(${target} "/OPT:ICF" RELEASE RELWITHDEBINFO)

	# disable manifests
	target_add_linker_flag(${target} "/MANIFEST:NO" RELEASE RELWITHDEBINFO)

	# enable function-level linking
	#target_add_compiler_flag(${target} "/Gy" RELEASE RELWITHDEBINFO)

	# sets the Checksum in the .exe header
	#target_add_linker_flag(${target} "/RELEASE" RELEASE RELWITHDEBINFO)

	# statically link C runtime library to static_runtime targets
	if(option_static_runtime)
		target_add_compiler_flag(${target} "/MTd" DEBUG)
		target_add_compiler_flag(${target} "/MT" RELWITHDEBINFO RELEASE)
	else()
		target_add_compiler_flag(${target} "/MDd" DEBUG)
		target_add_compiler_flag(${target} "/MD" RELWITHDEBINFO RELEASE)
	endif()

	# manage warnings
	set(flags)
	if(option_no_warnings)
		set(flags "/W0")
	elseif(option_low_warnings)
		set(flags "/W3")
	else()
		set(flags
		  ## Base flags:
		  "/W4"

		  ## Extra flags:
		  "/w44263" # 'function': member function does not override any base class virtual member function
		  "/w44265" # 'class': class has virtual functions, but destructor is not virtual
		  "/w44287" # 'operator': unsigned/negative constant mismatch
		  "/w44289" # nonstandard extension used : 'var' : loop control variable declared in the for-loop is used outside the for-loop scope
		  "/w44296" # 'operator': expression is always false
		  "/w44355" # 'this' : used in base member initializer list
		  "/w44365" # 'action': conversion from 'type_1' to 'type_2', signed/unsigned mismatch
		  "/w44412" # 'function': function signature contains type 'type'; C++ objects are unsafe to pass between pure code and mixed or native
		  "/w44431" # missing type specifier - int assumed. Note: C no longer supports default-int
		  "/w44536" # 'type name': type-name exceeds meta-data limit of 'limit' characters
		  "/w44545" # expression before comma evaluates to a function which is missing an argument list
		  "/w44546" # function call before comma missing argument list
		  "/w44547" # 'operator': operator before comma has no effect; expected operator with side-effect
		  "/w44548" # expression before comma has no effect; expected expression with side-effect
		  "/w44549" # 'operator': operator before comma has no effect; did you intend 'operator'?
		  "/w44555" # expression has no effect; expected expression with side-effect
		  "/w44619" # #pragma warning: there is no warning number 'number'
		  #"/w44623" # 'derived class': default constructor could not be generated because a base class default constructor is inaccessible
		  #"/w44625" # 'derived class': copy constructor could not be generated because a base class copy constructor is inaccessible
		  #"/w44626" # 'derived class': assignment operator could not be generated because a base class assignment operator is inaccessible
		  #"/w44640" # 'instance': construction of local static object is not thread-safe
		  "/w44917" # 'declarator': a GUID can only be associated with a class, interface, or namespace
		  "/w44946" # reinterpret_cast used between related classes: 'class1' and 'class2'
		  "/w44986" # 'symbol': exception specification does not match previous declaration
		  "/w44987" # nonstandard extension used: 'throw (...)'
		  "/w44988" # 'symbol': variable declared outside class/function scope
		  "/w45022" # 'type': multiple move constructors specified
		  "/w45023" # 'type': multiple move assignment operators specified
		  "/w45029" # nonstandard extension used: alignment attributes in C++ apply to variables, data members and tag types only
		  "/w45031" # #pragma warning(pop): likely mismatch, popping warning state pushed in different file
		  "/w45032" # detected #pragma warning(push) with no corresponding #pragma warning(pop)
		  "/w45034" # use of intrinsic 'intrinsic' causes function function to be compiled as guest code
		  "/w45035" # use of feature 'feature' causes function function to be compiled as guest code
		  "/w45036" # varargs function pointer conversion when compiling with /hybrid:x86arm64 'type1' to 'type2'
		  "/w45038" # data member 'member1' will be initialized after data member 'member2'
		  "/w45039" # 'function': pointer or reference to potentially throwing function passed to extern C function under -EHc. Undefined behavior may occur if this function throws an exception.
		  "/w45042" # 'function': function declarations at block scope cannot be specified 'inline' in standard C++; remove 'inline' specifier

		  ## Apocalypse flags:
		  #"/Wall"
		  #"/WX"
		  )
	endif()
	foreach(flag ${flags})
		target_add_compiler_flag(${target} ${flag})
	endforeach()
endfunction()

## setup_gcc(target [OPTIONS [static_runtime] [c] [cxx] [no_warnings] [low_warnings]])
# Set up gcc-specific options of the target.
# Without options: maximum warnings are enabled.
#   {value}  [in] target:           Target to configure
#   {option} [in] static_runtime:   If present, C runtime library is statically linked
#   {option} [in] c:                If present, the target is written in C, add warnings if no warnings option is specified
#   {option} [in] cxx:              If present, the target is written in C++, add warnings if no warnings option is specified
#   {option} [in] no_warnings:      If present, warnings are disabled (useful for external projects)
#   {option} [in] low_warnings:     If present, low/normal warnings are enabled
function(setup_gcc target)
	split_args(ignore "OPTIONS" options ${ARGN})
	has_item(option_static_runtime "static_runtime" ${options})
	has_item(option_c "c" ${options})
	has_item(option_cxx "cxx" ${options})
	has_item(option_no_warnings "no_warnings" ${options})
	has_item(option_low_warnings "low_warnings" ${options})

	# setup ccache
	find_program(CCACHE_PROGRAM ccache)
	if(NOT ${CCACHE_PROGRAM} STREQUAL CCACHE_PROGRAM-NOTFOUND)
		message(STATUS "ccache found: ${CCACHE_PROGRAM}")
		set_target_properties(${target} PROPERTIES C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
		set_target_properties(${target} PROPERTIES CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
		message(STATUS "Enabled ccache on target ${target}")
	endif()

	# generates complete debugging information
	target_add_compiler_flag(${target} "-g3" DEBUG RELWITHDEBINFO)

	# set optimization
	target_add_compiler_flag(${target} "-O0" DEBUG)
	target_add_compiler_flag(${target} "-O2" RELWITHDEBINFO)
	target_add_compiler_flag(${target} "-O3" RELEASE)

	# statically link C runtime library to static_runtime targets
	if(option_static_runtime)
		target_add_compiler_flag(${target} "-static-libgcc")
		target_add_compiler_flag(${target} "-static-libstdc++")
	endif()

	# enable sanitizers
	#target_add_linker_flag(${target} "-fsanitize=address" DEBUG RELWITHDEBINFO)
	#target_add_linker_flag(${target} "-fsanitize=thread" DEBUG RELWITHDEBINFO)
	#target_add_linker_flag(${target} "-fsanitize=memory" DEBUG RELWITHDEBINFO)
	#target_add_linker_flag(${target} "-fsanitize=undefined" DEBUG RELWITHDEBINFO)
	#target_add_linker_flag(${target} "-fsanitize=leak" DEBUG RELWITHDEBINFO)

	# enable libstdc++ "debug" mode
	# warning: changes the size of some standard class templates
	# you cannot pass containers between translation units compiled
	# with and without libstdc++ "debug" mode
	#target_add_compile_definition(${target} _GLIBCXX_DEBUG DEBUG)
	#target_add_compile_definition(${target} _GLIBCXX_DEBUG_PEDANTIC DEBUG)
	#target_add_compile_definition(${target} _GLIBCXX_SANITIZE_VECTOR DEBUG)

	# manage warnings
	set(flags)
	set(c_flags)
	set(cxx_flags)
	if(option_no_warnings)
		set(flags "--no-warnings")
	elseif(option_low_warnings)
		set(flags "-pedantic" "-Wall")
	else()
		set(flags
		  ## Base flags:
		  "-pedantic"
		  "-pedantic-errors"
		  "-Wall"
		  "-Wextra"

		  ## Extra flags:
		  "-Wdouble-promotion"
		  "-Wnull-dereference"
		  "-Wimplicit-fallthrough"
		  "-Wif-not-aligned"
		  "-Wmissing-include-dirs"
		  "-Wswitch-bool"
		  "-Wswitch-unreachable"
		  "-Walloc-zero"
		  "-Wduplicated-branches"
		  "-Wduplicated-cond"
		  "-Wfloat-equal"
		  "-Wshadow"
		  "-Wundef"
		  "-Wexpansion-to-defined"
		  #"-Wunused-macros"
		  "-Wcast-qual"
		  "-Wcast-align"
		  "-Wwrite-strings"
		  "-Wconversion"
		  "-Wsign-conversion"
		  "-Wdate-time"
		  "-Wextra-semi"
		  "-Wlogical-op"
		  "-Wmissing-declarations"
		  "-Wredundant-decls"
		  "-Wrestrict"
		  #"-Winline"
		  "-Winvalid-pch"
		  "-Woverlength-strings"
		  "-Wformat=2"
		  "-Wformat-signedness"
		  "-Winit-self"

		  ## Optimisation dependant flags
		  "-Wstrict-overflow=5"

		  ## Info flags
		  #"-Winvalid-pch"
		  #"-Wvolatile-register-var"
		  #"-Wdisabled-optimization"
		  #"-Woverlength-strings"
		  #"-Wunsuffixed-float-constants"
		  #"-Wvector-operation-performance"

		  ## Apocalypse flags:
		  #"-Wsystem-headers"
		  #"-Werror"

		  ## Exit on first error
		  "-Wfatal-errors"
		  )
		if(option_c)
			set(c_flags
			  "-Wdeclaration-after-statement"
			  "-Wbad-function-cast"
			  "-Wjump-misses-init"
			  "-Wstrict-prototypes"
			  "-Wold-style-definition"
			  "-Wmissing-prototypes"
			  "-Woverride-init-side-effects"
			  "-Wnested-externs"
			  #"-Wc90-c99-compat"
			  #"-Wc99-c11-compat"
			  #"-Wc++-compat"
			  )
		endif()
		if(option_cxx)
			set(cxx_flags
			  "-Wzero-as-null-pointer-constant"
			  "-Wsubobject-linkage"
			  "-Wdelete-incomplete"
			  "-Wuseless-cast"
			  "-Wctor-dtor-privacy"
			  "-Wnoexcept"
			  "-Wregister"
			  "-Wstrict-null-sentinel"
			  "-Wold-style-cast"
			  "-Woverloaded-virtual"

			  ## Lifetime
			  "-Wlifetime"

			  ## Suggestions
			  "-Wsuggest-override"
			  #"-Wsuggest-final-types"
			  #"-Wsuggest-final-methods"
			  #"-Wsuggest-attribute=pure"
			  #"-Wsuggest-attribute=const"
			  #"-Wsuggest-attribute=noreturn"
			  #"-Wsuggest-attribute=format"

			  ## Guidelines from Scott Meyers’ Effective C++ series of books
			  #"-Weffc++"

			  ## Special purpose
			  #"-Wsign-promo"
			  #"-Wtemplates"
			  #"-Wmultiple-inheritance"
			  #"-Wvirtual-inheritance"
			  #"-Wnamespaces"

			  ## Standard versions
			  #"-Wc++11-compat"
			  #"-Wc++14-compat"
			  #"-Wc++17-compat"
			  )
		endif()
	endif()
	foreach(flag IN ITEMS ${flags} ${c_flags} ${cxx_flags})
		target_add_compiler_flag(${target} ${flag})
	endforeach()
endfunction()

## setup_clang(target [OPTIONS [static_runtime] [no_warnings] [low_warnings]])
# Set up clang-specific options of the target.
# Without options: maximum warnings are enabled.
#   {value}  [in] target:           Target to configure
#   {option} [in] static_runtime:   If present, C runtime library is statically linked
#   {option} [in] no_warnings:      If present, warnings are disabled (useful for external projects)
#   {option} [in] low_warnings:     If present, low/normal warnings are enabled
function(setup_clang target)
	split_args(ignore "OPTIONS" options ${ARGN})
	has_item(option_static_runtime "static_runtime" ${options})
	has_item(option_no_warnings "no_warnings" ${options})
	has_item(option_low_warnings "low_warnings" ${options})

	# setup ccache
	find_program(CCACHE_PROGRAM ccache)
	if(NOT ${CCACHE_PROGRAM} STREQUAL CCACHE_PROGRAM-NOTFOUND)
		message(STATUS "ccache found: ${CCACHE_PROGRAM}")
		set_target_properties(${target} PROPERTIES C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
		set_target_properties(${target} PROPERTIES CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
		message(STATUS "Enabled ccache on target ${target}")
	endif()

	# generates complete debugging information
	target_add_compiler_flag(${target} "-g3" DEBUG RELWITHDEBINFO)

	# set optimization
	target_add_compiler_flag(${target} "-O0" DEBUG)
	target_add_compiler_flag(${target} "-O2" RELWITHDEBINFO)
	target_add_compiler_flag(${target} "-O3" RELEASE)

	# statically link C runtime library to static_runtime targets
	if(option_static_runtime)
		target_add_compiler_flag(${target} "-static-libgcc")
		target_add_compiler_flag(${target} "-static-libstdc++")
	endif()

	# enable sanitizers
	#target_add_linker_flag(${target} "-fsanitize=address" DEBUG RELWITHDEBINFO)
	#target_add_linker_flag(${target} "-fsanitize=thread" DEBUG RELWITHDEBINFO)
	#target_add_linker_flag(${target} "-fsanitize=memory" DEBUG RELWITHDEBINFO)
	#target_add_linker_flag(${target} "-fsanitize=undefined" DEBUG RELWITHDEBINFO)
	#target_add_linker_flag(${target} "-fsanitize=leak" DEBUG RELWITHDEBINFO)

	# enable libstdc++ "debug" mode
	# warning: changes the size of some standard class templates
	# you cannot pass containers between translation units compiled
	# with and without libstdc++ "debug" mode
	#target_add_compile_definition(${target} _GLIBCXX_DEBUG DEBUG)
	#target_add_compile_definition(${target} _GLIBCXX_DEBUG_PEDANTIC DEBUG)
	#target_add_compile_definition(${target} _GLIBCXX_SANITIZE_VECTOR DEBUG)

	# manage warnings
	set(flags)
	if(option_no_warnings)
		set(flags "-Wno-everything")
	elseif(option_low_warnings)
		set(flags "-pedantic" "-Wall")
	else()
		set(flags
		  ## Base flags:
		  "-pedantic"
		  "-pedantic-errors"
		  "-Wall"
		  "-Wextra"

		  ## Extra flags:
		  "-Wbad-function-cast"
		  "-Wcomplex-component-init"
		  "-Wconditional-uninitialized"
		  #"-Wcovered-switch-default"
		  "-Wcstring-format-directive"
		  "-Wdelete-non-virtual-dtor"
		  "-Wdeprecated"
		  "-Wdollar-in-identifier-extension"
		  "-Wdouble-promotion"
		  "-Wduplicate-enum"
		  "-Wduplicate-method-arg"
		  "-Wembedded-directive"
		  "-Wexpansion-to-defined"
		  "-Wextended-offsetof"
		  "-Wfloat-conversion"
		  #"-Wfloat-equal"
		  "-Wfor-loop-analysis"
		  "-Wformat-pedantic"
		  "-Wgnu"
		  "-Wimplicit-fallthrough"
		  "-Winfinite-recursion"
		  "-Winvalid-or-nonexistent-directory"
		  "-Wkeyword-macro"
		  "-Wmain"
		  "-Wmethod-signatures"
		  "-Wmicrosoft"
		  "-Wmismatched-tags"
		  "-Wmissing-field-initializers"
		  "-Wmissing-method-return-type"
		  "-Wmissing-prototypes"
		  "-Wmissing-variable-declarations"
		  "-Wnested-anon-types"
		  "-Wnon-virtual-dtor"
		  "-Wnonportable-system-include-path"
		  "-Wnull-pointer-arithmetic"
		  "-Wnullability-extension"
		  "-Wold-style-cast"
		  "-Woverriding-method-mismatch"
		  "-Wpacked"
		  "-Wpedantic"
		  "-Wpessimizing-move"
		  "-Wredundant-move"
		  "-Wreserved-id-macro"
		  "-Wself-assign"
		  "-Wself-move"
		  "-Wsemicolon-before-method-body"
		  "-Wshadow"
		  "-Wshadow-field"
		  "-Wshadow-field-in-constructor"
		  "-Wshadow-uncaptured-local"
		  "-Wshift-sign-overflow"
		  "-Wshorten-64-to-32"
		  #"-Wsign-compare"
		  #"-Wsign-conversion"
		  "-Wsigned-enum-bitfield"
		  "-Wstatic-in-inline"
		  #"-Wstrict-prototypes"
		  #"-Wstring-conversion"
		  #"-Wswitch-enum"
		  "-Wtautological-compare"
		  "-Wtautological-overlap-compare"
		  "-Wthread-safety"
		  "-Wundefined-reinterpret-cast"
		  "-Wuninitialized"
		  #"-Wunknown-pragmas"
		  "-Wunreachable-code"
		  "-Wunreachable-code-aggressive"
		  #"-Wunused"
		  "-Wunused-const-variable"
		  "-Wunused-lambda-capture"
		  "-Wunused-local-typedef"
		  "-Wunused-parameter"
		  "-Wunused-private-field"
		  "-Wunused-template"
		  "-Wunused-variable"
		  "-Wused-but-marked-unused"
		  "-Wzero-as-null-pointer-constant"
		  "-Wzero-length-array"

		  ## Lifetime
		  "-Wlifetime"

		  ## Info flags
		  "-Wcomma"
		  "-Wcomment"

		  ## Exit on first error
		  "-Wfatal-errors"
		  )
	endif()
	foreach(flag ${flags})
		target_add_compiler_flag(${target} ${flag})
	endforeach()
endfunction()

## setup_target(target [OPTIONS [options...]])
# Set up options of the target depending of the compiler.
# To know possible option, see the setup_<COMPILER> functions documentation.
# Currently supported compilers: msvc, gcc.
#   {value}  [in] target:    Target to configure
#   {option} [in] options:   Configuration options
function(setup_target target)
	if(MSVC)
		setup_msvc(${target} OPTIONS ${ARGN})
	elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
		setup_gcc(${target} OPTIONS ${ARGN})
	elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
		setup_clang(${target} OPTIONS ${ARGN})
	else()
		message(WARNING "Unsupported compiler (${CMAKE_CXX_COMPILER_ID}) setup")
	endif()
endfunction()

## make_target(target group files... [INCLUDES includes...] [EXT_INCLUDES ext_includes...]
##     [OPTIONS [executable] [test] [shared] [static_runtime] [c] [cxx] [no_warnings] [low_warnings]])
# Make a new target with the input options
# By default targets are static libraries.
#   {value}  [in] target:           Target name
#   {value}  [in] group:            Group of the target (can contain '/'' for subgroups)
#   {value}  [in] files:            Source files
#   {value}  [in] includes:         Include files
#   {value}  [in] ext_includes:     External include files (no warnings)
#   {option} [in] executable:       If present, build an executable
#   {option} [in] test:             If present, build a test executable
#   {option} [in] shared:           If present, build a shared library
#   {option} [in] static_runtime:   If present, C runtime library is statically linked
#   {option} [in] c:                If present, the target is written in C, add warnings if no warnings option is specified
#   {option} [in] cxx:              If present, the target is written in C++, add warnings if no warnings option is specified
#   {option} [in] no_warnings:      If present, warnings are disabled (useful for external projects)
#   {option} [in] low_warnings:     If present, low/normal warnings are enabled
function(make_target target group)
	message(STATUS "Configuring ${group}/${target}")

	# initialize additional include directory list
	set(includes)
	set(ext_includes)

	# get options
	split_args(inputs "OPTIONS" options ${ARGN})
	split_args(inputs2 "EXT_INCLUDES" ext_includes ${inputs})
	split_args(files "INCLUDES" includes ${inputs2})
	has_item(is_executable "executable" ${options})
	has_item(is_test "test" ${options})
	has_item(is_shared "shared" ${options})

	# sort files
	list(SORT files)

	# add the target
	if(is_executable OR is_test)
		add_executable(${target} "" ${files})
		if(is_test)
			add_test(NAME ${target} COMMAND ${target})
			#add_custom_command(TARGET ${target} POST_BUILD COMMAND $<TARGET_FILE:${target}>)
		endif()
	elseif(is_shared)
		add_library(${target} SHARED ${files})
	else()
		add_library(${target} STATIC ${files})
	endif()

	# setup compiler dependent options
	setup_target(${target} ${options})

	# add all additional include directories
	target_add_includes(${target} ${includes})
	target_add_system_includes(${target} ${ext_includes})

	# set directories for IDE
	source_group(CMake REGULAR_EXPRESSION ".*[.](cmake|rule)$")
	source_group(CMake FILES "fmt.cmake")
	set_target_properties(${target} PROPERTIES FOLDER ${group})
endfunction()

## configure_folder(input_folder output_folder [args...])
# Recursively copy all files from an input folder to an output folder
# Copy is made with CMake configure_file(), see documentation for more information:
# https://cmake.org/cmake/help/latest/command/configure_file.html
#   {value} [in] input_folder:    Input folder
#   {value} [in] output_folder:   Output folder
#   {value} [in] args:            CMake configure_file() additional arguments
function(configure_folder input_folder output_folder)
	file(GLOB_RECURSE files "${input_folder}/*")
	foreach(file ${files})
		file(RELATIVE_PATH relative_file ${input_folder} ${file})
		configure_file(${file} "${output_folder}/${relative_file}" ${ARGN})
	endforeach()
endfunction()


# disable compiler specific extensions
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

# set C_STANDARD/CXX_STANDARD as a requirement
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# don't allow to build in sources otherwise a makefile not generated by CMake can be overridden
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# place generated binaries in build/bin
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/build/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/build/bin/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/build/bin/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/build/bin/)

# place generated libs in build/lib
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/build/lib/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/build/lib/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/build/lib/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/build/lib/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/build/lib/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/build/lib/)

# enable IDE folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "_CMake")
