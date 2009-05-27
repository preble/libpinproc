cmake_minimum_required(VERSION 2.6)

#set( CMAKE_OSX_ARCHITECTURES ppc;i386 ) # Uncomment for universal binary

#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../bin)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../bin)

include_directories(/usr/local/include $ENV{EXTRA_INC})

link_directories(/usr/local/lib $ENV{EXTRA_LINK})

set(FILES src/pinproc.cpp src/PRDevice.cpp src/PRHardware.cpp)
add_library(pinproc ${FILES})

add_executable(pinproctest examples/pinproctest/pinproctest.cpp)
target_link_libraries(pinproctest pinproc usb ftdi yaml-cpp)

