###
### CMake policy settings
###
# see http://www.cmake.org/Wiki/CMake_Policies
cmake_minimum_required(VERSION 2.6)
# see http://www.cmake.org/cmake/help/cmake-2-8-docs.html#policy:CMP0015
if(POLICY CMP0015)
	cmake_policy(SET CMP0015 NEW)
endif()


###
### Project settings
###
project(PINPROC)

set(PINPROC_VERSION_MAJOR "0")
set(PINPROC_VERSION_MINOR "9")
set(PINPROC_VERSION "${PINPROC_VERSION_MAJOR}.${PINPROC_VERSION_MINOR}")


###
### Project options
###
# General stuff
option(PINPROC_BUILD_TOOLS "Enable testing and firmware tools" ON)

# Compilation options
# see http://www.cmake.org/cmake/help/cmake2.6docs.html#variable:BUILD_SHARED_LIBS
#     http://www.cmake.org/cmake/help/cmake2.6docs.html#command:add_library
option(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
# see http://msdn.microsoft.com/en-us/library/aa278396(v=vs.60).aspx
#     http://msdn.microsoft.com/en-us/library/2kzt1wy3%28v=VS.71%29.aspx
option(MSVC_SHARED_RT "MSVC: Build with shared runtime libs (/MD)" ON)
option(MSVC_STHREADED_RT "MSVC: Build with single-threaded static runtime libs (/ML until VS .NET 2003)" OFF)


###
### Source
###
# !!! file(GLOB) didn't work on Ubuntu 10.04 with cmake 2.8 !!!
#file(GLOB sources src/[a-z]*.cpp)
#file(GLOB public_headers include/[a-z]*.h)
#file(GLOB private_headers src/[a-z]*.h)
set(sources "src/pinproc.cpp;src/PRDevice.cpp;src/PRHardware.cpp")
set(public_headers "include/pinproc.h")
set(private_headers "src/PRCommon.h;src/PRDevice.h;src/PRHardware.h")
message(STATUS "sources is ${sources}")
message(STATUS "public_headers is ${public_headers}")
message(STATUS "private_headers is ${private_headers}")

include_directories(${PINPROC_SOURCE_DIR}/include $ENV{EXTRA_INC} /usr/local/include)
link_directories($ENV{EXTRA_LINK} /usr/local/lib)


###
### General compilation settings
###
if(APPLE)
#	set(CMAKE_OSX_ARCHITECTURES ppc;i386)	# Uncomment for universal binary
endif()

if(WIN32)
	set(libraries ftd2xx)
	if(BUILD_SHARED_LIBS)
		add_definitions(-DPINPROC_DLL)	# use or build Windows DLL
	endif()
else()
	set(libraries usb ftdi)
endif()

set(YAML_CPP_LIB "yaml-cpp")
set(YAML_CPP_LIB_DBG "${YAML_CPP_LIB}")


###
### General install settings
###
if(WIN32)
	set(_library_dir bin)	# .dll are in PATH, like executables
else()
	set(_library_dir lib)
endif()

set(INCLUDE_INSTALL_DIR include/p-roc)
set(LIB_INSTALL_DIR ${_library_dir}${LIB_SUFFIX})

set(_INSTALL_DESTINATIONS
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION ${LIB_INSTALL_DIR}
	ARCHIVE DESTINATION lib${LIB_SUFFIX}
)


###
### Library
###
add_library(pinproc
	${sources}
	${public_headers}
	${private_headers}
)
set_target_properties(pinproc PROPERTIES
	VERSION "${PINPROC_VERSION}"
	SOVERSION "${PINPROC_VERSION_MAJOR}.${PINPROC_VERSION_MINOR}"
)

# Correct MSVC for static libraries (/MD[d], /MT[d], /ML[d] (single-threaded until VS 2003))
# see http://msdn.microsoft.com/en-us/library/aa278396(v=vs.60).aspx
#     http://msdn.microsoft.com/en-us/library/2kzt1wy3%28v=VS.71%29.aspx
if((MSVC) AND (NOT BUILD_SHARED_LIBS))
	set(TMP_SUFFIX "md")	# CMake defaults to /MD for MSVC

	if (NOT MSVC_SHARED_RT)	# User wants to have static runtime libraries (/MT, /ML)
		if (MSVC_STHREADED_RT)	# User wants to have old single-threade runtime libraries
			set(TMP_SUFFIX "ml")
			set(TMP_OPTION "/ML")
			if(NOT MSVC_VERSION LESS 1400)
				message(FATAL_ERROR "Single-threaded runtime libraries (/ML) only availbe until VS .NET 2003 (7.1).")
			endif()
		else()
			set(TMP_SUFFIX "mt")
			set(TMP_OPTION "/MT")
		endif()

		# correct linker options
		string(REPLACE "/MD" "${TMP_OPTION}" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
		string(REPLACE "/MD" "${TMP_OPTION}" CMAKE_CXX_FLAGS_MINSIZEREL ${CMAKE_CXX_FLAGS_MINSIZEREL})
		string(REPLACE "/MD" "${TMP_OPTION}" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
		string(REPLACE "/MD" "${TMP_OPTION}" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
		string(REPLACE "/MD" "${TMP_OPTION}" CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
		string(REPLACE "/MD" "${TMP_OPTION}" CMAKE_C_FLAGS_MINSIZEREL ${CMAKE_C_FLAGS_MINSIZEREL})
		string(REPLACE "/MD" "${TMP_OPTION}" CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
		string(REPLACE "/MD" "${TMP_OPTION}" CMAKE_C_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO})
	endif()

	# correct library names
	set_target_properties(pinproc PROPERTIES PREFIX "lib")
	set_target_properties(pinproc PROPERTIES DEBUG_POSTFIX "${TMP_SUFFIX}d")
	set_target_properties(pinproc PROPERTIES RELEASE_POSTFIX "${TMP_SUFFIX}")
	set_target_properties(pinproc PROPERTIES MINSIZEREL_POSTFIX "${TMP_SUFFIX}")
	set_target_properties(pinproc PROPERTIES RELWITHDEBINFO_POSTFIX "${TMP_SUFFIX}")
	# correct external library names
	set(YAML_CPP_LIB "lib${YAML_CPP_LIB}${TMP_SUFFIX}")
	set(YAML_CPP_LIB_DBG "${YAML_CPP_LIB}d")
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
#TODO: use add_subdirectory() and separate CMakeLists.txt
if(PINPROC_BUILD_TOOLS)
# Create a target for the test tool (see yaml-cpp)
add_executable(pinproctest
	examples/pinproctest/pinproctest.cpp
	examples/pinproctest/drivers.cpp
	examples/pinproctest/dmd.cpp
	examples/pinproctest/switches.cpp
	examples/pinproctest/alphanumeric.cpp
)
target_link_libraries(pinproctest
	pinproc
	${libraries}
	optimized ${YAML_CPP_LIB}
	debug ${YAML_CPP_LIB_DBG}
)

# Create a target for the firmware tool (see yaml-cpp)
#TODO: use add_subdirectory() and separate CMakeLists.txt
add_executable(pinprocfw
	utils/pinprocfw/pinprocfw.cpp
	utils/pinprocfw/lenval.cpp
)
target_link_libraries(pinprocfw
	pinproc
	${libraries}
)
endif()
