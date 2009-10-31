cmake_minimum_required(VERSION 2.6)

#set( CMAKE_OSX_ARCHITECTURES ppc;i386 ) # Uncomment for universal binary

#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../bin)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../bin)

include_directories(/usr/local/include $ENV{EXTRA_INC})

link_directories(/usr/local/lib $ENV{EXTRA_LINK})

set(FILES src/pinproc.cpp src/PRDevice.cpp src/PRHardware.cpp)
add_library(pinproc ${FILES})

if(WIN32)
  set(libraries ftd2xx)
else(WIN32)
  set(libraries usb ftdi)
endif(WIN32)

add_executable(pinproctest examples/pinproctest/pinproctest.cpp examples/pinproctest/drivers.cpp examples/pinproctest/dmd.cpp examples/pinproctest/switches.cpp)
add_executable(pinprocfw utils/pinprocfw/pinprocfw.cpp utils/pinprocfw/lenval.cpp)
target_link_libraries(pinproctest pinproc ${libraries} yaml-cpp)
target_link_libraries(pinprocfw pinproc ${libraries})
