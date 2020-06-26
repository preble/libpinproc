###
### CMake settings
###
## Due to Mac OSX we need to keep compatibility with CMake 2.6
# see http://www.cmake.org/Wiki/CMake_Policies
cmake_minimum_required(VERSION 2.6)
# see http://www.cmake.org/cmake/help/cmake-2-8-docs.html#policy:CMP0012
if(POLICY CMP0012)
	cmake_policy(SET CMP0012 OLD)
endif()
# see http://www.cmake.org/cmake/help/cmake-2-8-docs.html#policy:CMP0015
if(POLICY CMP0015)
	cmake_policy(SET CMP0015 OLD)
endif()

# allow relative paths in LINK_DIRECTORIES
if(POLICY CMP0081)
	cmake_policy(SET CMP0081 OLD)
endif()

###
### Project settings
###
project(PINPROC)

set(PINPROC_VERSION_MAJOR "2")
set(PINPROC_VERSION_MINOR "0")
set(PINPROC_VERSION "${PINPROC_VERSION_MAJOR}.${PINPROC_VERSION_MINOR}")


###
### Project options
###
## Project stuff
option(PINPROC_BUILD_TOOLS "Enable testing and firmware tools" ON)

## Build options
# --> General
# see http://www.cmake.org/cmake/help/cmake2.6docs.html#variable:BUILD_SHARED_LIBS
#     http://www.cmake.org/cmake/help/cmake2.6docs.html#command:add_library
option(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)

# --> Apple
option(APPLE_UNIVERSAL_BIN "Apple: Build universal binary" OFF)

# --> Microsoft Visual C++
# see http://msdn.microsoft.com/en-us/library/aa278396(v=VS.60).aspx
#     http://msdn.microsoft.com/en-us/library/2kzt1wy3(v=VS.71).aspx
option(MSVC_SHARED_RT "MSVC: Build with shared runtime libs (/MD)" ON)
option(MSVC_STHREADED_RT "MSVC: Build with single-threaded static runtime libs (/ML until VS .NET 2003)" OFF)

option(CROSS_ROOT "Cross-compilation root path" OFF)

###
### Sources, headers, directories and libs
###
file(GLOB sources "src/[a-zA-Z]*.cpp")
file(GLOB public_headers "include/[a-zA-Z]*.h")
file(GLOB private_headers "src/[a-zA-Z]*.h")

if(WIN32)
	if(BUILD_SHARED_LIBS)
		set(defs
			src/pinproc.def
		)
	endif()
endif()

if(VERBOSE)
	message(STATUS "sources: ${sources}")
	message(STATUS "public_headers: ${public_headers}")
	message(STATUS "private_headers: ${private_headers}")
	message(STATUS "defs: ${defs}")
endif()

# use -DEXTRA_INC="<path>;<path>" and -DEXTRA_LINK="<path>;<path>"
set(EXTRA_INC "" CACHE STRING "Extra include directories separated by ;")
set(EXTRA_LINK "" CACHE STRING "Extra link directories separated by ;")
if(CROSS_ROOT)
include_directories(${PINPROC_SOURCE_DIR}/include ${EXTRA_INC} ${CROSS_ROOT}/usr/local/include)
link_directories(${EXTRA_LINK} ${CROSS_ROOT}/usr/local/lib)
else()
include_directories(${PINPROC_SOURCE_DIR}/include ${EXTRA_INC} /usr/local/include)
link_directories(${EXTRA_LINK} /usr/local/lib)
endif()


###
### General compilation settings
###
if(BUILD_SHARED_LIBS)
	set(LABEL_SUFFIX "shared")
else()
	set(LABEL_SUFFIX "static")
endif()

if(APPLE)
	if(APPLE_UNIVERSAL_BIN)
		set(CMAKE_OSX_ARCHITECTURES ppc;i386)
	endif()
endif()

if(WIN32)
	set(lib_ftdi_usb ftd2xx)
	if(BUILD_SHARED_LIBS)
		add_definitions(-D${PROJECT_NAME}_DLL)	# use or build Windows DLL
	endif()
	if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		set(CMAKE_INSTALL_PREFIX "C:/")
	endif()
else()
	set(lib_ftdi_usb usb ftdi1)
endif()

# GCC specialities
if(CMAKE_COMPILER_IS_GNUCC)
	if(WIN32)
		set(CMAKE_SHARED_LIBRARY_PREFIX "")	# DLLs do not have a "lib" prefix
		set(CMAKE_IMPORT_LIBRARY_PREFIX "")	# same for DLL import libs
		set(CMAKE_LINK_DEF_FILE_FLAG "")	# CMake workaround (2.8.3)
	endif()
endif()

# Microsoft VisualC++ specialities
if(MSVC)
	### General stuff
	# a) Change MSVC runtime library settings (/MD[d], /MT[d], /ML[d] (single-threaded until VS 2003))
	#    plus set lib suffix for later use and project label accordingly
	# see http://msdn.microsoft.com/en-us/library/aa278396(v=VS.60).aspx
	#     http://msdn.microsoft.com/en-us/library/2kzt1wy3(v=VS.71).aspx
	set(LIB_RT_SUFFIX "md")	# CMake defaults to /MD for MSVC
	set(LIB_RT_OPTION "/MD")
	#
	if(NOT MSVC_SHARED_RT)	# User wants to have static runtime libraries (/MT, /ML)
		if(MSVC_STHREADED_RT)	# User wants to have old single-threaded static runtime libraries
			set(LIB_RT_SUFFIX "ml")
			set(LIB_RT_OPTION "/ML")
			if(NOT ${MSVC_VERSION} LESS 1400)
				message(FATAL_ERROR "Single-threaded static runtime libraries (/ML) only available until VS .NET 2003 (7.1).")
			endif()
		else()
			set(LIB_RT_SUFFIX "mt")
			set(LIB_RT_OPTION "/MT")
		endif()

		# correct linker options
		foreach(flag_var  CMAKE_C_FLAGS  CMAKE_CXX_FLAGS)
			foreach(config_name  ""  DEBUG  RELEASE  MINSIZEREL  RELWITHDEBINFO)
				set(var_name "${flag_var}")
				if(NOT "${config_name}" STREQUAL "")
					set(var_name "${var_name}_${config_name}")
				endif()
				string(REPLACE "/MD" "${LIB_RT_OPTION}" ${var_name} "${${var_name}}")
			endforeach()
		endforeach()
	endif()
	#
	set(LABEL_SUFFIX "${LABEL_SUFFIX} ${LIB_RT_SUFFIX}")

	# b) Change prefix for static libraries
	set(CMAKE_STATIC_LIBRARY_PREFIX "lib")	# to distinguish static libraries from DLL import libs

	# c) Correct suffixes for static libraries
	if(NOT BUILD_SHARED_LIBS)
		### General stuff
		set(LIB_TARGET_SUFFIX "${LIB_SUFFIX}${LIB_RT_SUFFIX}")
	endif()
endif()


###
### General install settings
###
if(WIN32)
	set(_library_dir bin)	# .dll are in PATH, like executables
else()
	set(_library_dir lib)
endif()

set(INCLUDE_INSTALL_DIR include/p-roc)
set(LIB_INSTALL_DIR "${_library_dir}${LIB_SUFFIX}")

set(_INSTALL_DESTINATIONS
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION ${LIB_INSTALL_DIR}
	ARCHIVE DESTINATION "lib${LIB_SUFFIX}"
)


###
### Library
###
add_library(pinproc
	${sources}
	${public_headers}
	${private_headers}
	${defs}
)

set_target_properties(pinproc PROPERTIES
	VERSION "${PINPROC_VERSION}"
	SOVERSION "${PINPROC_VERSION_MAJOR}.${PINPROC_VERSION_MINOR}"
	PROJECT_LABEL "pinproc ${LABEL_SUFFIX}"
)

target_link_libraries(pinproc
	${lib_ftdi_usb}
)

if(MSVC)
	if(NOT BUILD_SHARED_LIBS)
		# correct library names
		set_target_properties(pinproc PROPERTIES
			DEBUG_POSTFIX "${LIB_TARGET_SUFFIX}d"
			RELEASE_POSTFIX "${LIB_TARGET_SUFFIX}"
			MINSIZEREL_POSTFIX "${LIB_TARGET_SUFFIX}"
			RELWITHDEBINFO_POSTFIX "${LIB_TARGET_SUFFIX}"
		)
	endif()
endif()

install(TARGETS pinproc ${_INSTALL_DESTINATIONS})
install(
	FILES ${public_headers}
	DESTINATION ${INCLUDE_INSTALL_DIR}
)

if(UNIX)
	set(PC_FILE ${CMAKE_BINARY_DIR}/pinproc.pc)
	configure_file("pinproc.pc.cmake" ${PC_FILE} @ONLY)
	install(FILES ${PC_FILE} DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
endif()


###
### Extras
###
if(PINPROC_BUILD_TOOLS)
# Create a target for the test tool
#TODO: use add_subdirectory() and separate CMakeLists.txt (like yaml-cpp)
#      see http://www.cmake.org/cmake/help/cmake-2-8-docs.html#command:add_subdirectory
add_executable(pinproctest
	examples/pinproctest/pinproctest.cpp
	examples/pinproctest/drivers.cpp
	examples/pinproctest/dmd.cpp
	examples/pinproctest/switches.cpp
	examples/pinproctest/alphanumeric.cpp
)
target_link_libraries(pinproctest
	pinproc
)

# Create a target for the firmware tool
#TODO: use add_subdirectory() and separate CMakeLists.txt (like yaml-cpp)
#      see http://www.cmake.org/cmake/help/cmake-2-8-docs.html#command:add_subdirectory
add_executable(pinprocfw
	utils/pinprocfw/pinprocfw.cpp
	utils/pinprocfw/lenval.cpp
)
target_link_libraries(pinprocfw
	pinproc
)
endif()
