cmake_minimum_required(VERSION 2.6)

project (LIBPINPROC)

set(PINPROC_VERSION_MAJOR "0")
set(PINPROC_VERSION_MINOR "0")
set(PINPROC_VERSION "${YAML_CPP_VERSION_MAJOR}.${YAML_CPP_VERSION_MINOR}")

IF(APPLE)
#  set( CMAKE_OSX_ARCHITECTURES ppc;i386 ) # Uncomment for universal binary
ENDIF(APPLE)

include_directories(${LIBPINPROC_SOURCE_DIR} /usr/local/include $ENV{EXTRA_INC})

link_directories(/usr/local/lib $ENV{EXTRA_LINK})

set(FILES src/pinproc.cpp src/PRDevice.cpp src/PRHardware.cpp)
add_library(pinproc ${FILES}
	${LIB_TYPE}
	${public_headers}
	${private_headers}
	${sources}
)
set_target_properties(pinproc PROPERTIES
	VERSION "${PINPROC_VERSION}"
	SOVERSION "${PINPROC_VERSION_MAJOR}.${PINPROC_VERSION_MINOR}"
)

if(WIN32)
  set(libraries ftd2xx)
  set(_library_dir bin) # .dll are in PATH, like executables
else(WIN32)
  set(libraries usb ftdi)
  set(_library_dir lib)
endif(WIN32)

add_executable(pinproctest examples/pinproctest/pinproctest.cpp examples/pinproctest/drivers.cpp examples/pinproctest/dmd.cpp examples/pinproctest/switches.cpp examples/pinproctest/alphanumeric.cpp)
add_executable(pinprocfw utils/pinprocfw/pinprocfw.cpp utils/pinprocfw/lenval.cpp)
target_link_libraries(pinproctest pinproc ${libraries} yaml-cpp)
target_link_libraries(pinprocfw pinproc ${libraries})

set(INCLUDE_INSTALL_DIR include)
set(LIB_INSTALL_DIR ${_library_dir}${LIB_SUFFIX})

#
set(_INSTALL_DESTINATIONS
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION ${LIB_INSTALL_DIR}
	ARCHIVE DESTINATION lib${LIB_SUFFIX}
)

file(GLOB public_headers include/[a-z]*.h)
file(GLOB private src/[a-z]*.h)
file(GLOB sources src/[a-z]*.cpp)

install(TARGETS pinproc ${_INSTALL_DESTINATIONS})
install(
	FILES ${public_headers}
	DESTINATION ${INCLUDE_INSTALL_DIR}
)

if(UNIX)
	set(PC_FILE ${CMAKE_BINARY_DIR}/pinproc.pc)
	configure_file("pinproc.pc.cmake" ${PC_FILE} @ONLY)
	install(FILES ${PC_FILE} DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
endif(UNIX)


#install(TARGETS pinproc 
#        ARCHIVE DESTINATION lib)
#set (HEADERS include/pinproc.h)
#install(FILES ${HEADERS} DESTINATION include)
