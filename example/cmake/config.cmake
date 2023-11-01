##/////////////////////////////////////
##Kasper de Bruin//////////////////////
##///26-10-23//////////////////////////
##/////////////////////////////////////
# Added support for AppleClang

# detect the OS
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	set(CONFIG_OS_WINDOWS 1)
	if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
		set(CONFIG_ARCH_32BITS 1)
	elseif(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
		set(CONFIG_ARCH_64BITS 1)
	else()
		message(FATAL_ERROR "Unsupported architecture")
		return()
	endif()
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(CONFIG_OS_UNIX 1)
	if(ANDROID)
		set(CONFIG_OS_ANDROID 1)
	else()
		set(CONFIG_OS_LINUX 1)
	endif()
elseif(CMAKE_SYSTEM_NAME MATCHES "^k?FreeBSD$")
	set(CONFIG_OS_FREEBSD 1)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	if(IOS)
		set(CONFIG_OS_IOS 1)
	else()
		set(CONFIG_OS_MACOSX 1)
	endif()
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
	set(CONFIG_OS_ANDROID 1)
else()
	message(FATAL_ERROR "Unsupported operating system or environment")
	return()
endif()


# detect the compiler and its version
if(CMAKE_CXX_COMPILER MATCHES ".*clang[+][+]" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(CONFIG_COMPILER_CLANG 1)

elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
	set(CONFIG_COMPILER_CLANG 1)
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	set(CONFIG_COMPILER_GCC 1)
elseif(MSVC)
	set(CONFIG_COMPILER_MSVC 1)
else()
	message(FATAL_ERROR "Unsupported compiler")
	return()
endif()
