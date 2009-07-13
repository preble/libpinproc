## libpinproc

Library for Gerry Stellenberg's [P-ROC](http://pinballcontrollers.com/) (Pinball Remote Operations Controller).

### Compiling

#### Prerequisites

libpinproc requires:

- [libusb-0.1.12](http://libusb.wiki.sourceforge.net/): Install with the default /usr/local prefix.  Version 0.1.12 has been tested on Mac and Linux.  Mac users: If you want to use libpinproc under Cocoa or pygame, you may wish to try libusb 1.0.  See below.

- [libftdi-0.16](http://www.intra2net.com/en/developer/libftdi/): Install with the default /usr/local prefix.

The pinproctest example requires [yaml-cpp](http://code.google.com/p/yaml-cpp/). Follow the build instructions, creating the build subdirectory.  After building, from the main source directory, run the following commands to manually install it:

    sudo cp lib/libyaml-cpp.a /usr/local/lib/
    sudo mkdir /usr/local/include/yaml-cpp
    sudo cp include/*.h /usr/local/include/yaml-cpp/

##### libusb-1.0 and libusb-compat

Version 1.0.2 does not work out of the box since libftdi is written against libusb-0.1.  You can use the libusb-compat-0.1.2 project, however, which creates a library that provides the older libusb interface.  Because Macs do not come with pkg-config, you may need to run configure for libusb-compat as follows:

    ./configure LIBUSB_1_0_CFLAGS=-I/usr/local/include/libusb-1.0 LIBUSB_1_0_LIBS="-L/usr/local/lib -lusb-1.0"

Note that libusb-1.0 must have been built and installed prior to this step.  This also assumes that you installed libusb-1.0 with the default /usr/local prefix.

#### Building with CMake

Download and install [CMake](http://www.cmake.org/cmake/resources/software.html).  Then:

    cd libpinproc
    mkdir build; cd build
    cmake ..
    make

The CMakeLists.txt file is presently designed to be run from a directory inside the libpinproc directory.  This will build both libpinproc and pinproctest.  Binaries will be placed in the directory that make was run from.

#### Building in Windows with minGW/CMake

Download and install [ftd2xx for Windows](http://www.ftdichip.com/Drivers/D2XX.htm).  It's recommended to use the [setup executable](http://www.ftdichip.com/Drivers/CDM/CDM%202.04.16.exe) for the driver install and the [zip file](http://www.ftdichip.com/Drivers/CDM/CDM%202.04.16%20WHQL%20Certified.zip) for the build source/dlls.

Download and install [CMake](http://www.cmake.org/cmake/resources/software.html). 

Download and install [MinGW](http://sourceforge.net/projects/mingw/files/). (Tested with MinGW 5.1.4)

Follow directions above for building yaml-cpp with the following exception:
 add '-G "MinGW Makefiles' to the cmake command line.

To build libpinproc:
 edit CMakeLists.txt:
  add "ftd2xx" to the target_link_libraries lines,
  remove "usb" and "ftdi" from the target_link_libraries lines

Until an automatic build process/structure is worked out:
 copy ftd2xx.h to libpinproc/src,
 copy yaml header files to libpinproc/examples/pinproctest,
 change libpinproc/examples/pinproctest/pinproctest.h to look for yaml.h locally

Follow instructions above for building libpinproc with cmake with the following exceptions:
 add '-G "MinGW Makefiles' to the cmake command line,
 use mingw-make instead of make
 
### License

Copyright (c) 2009 Gerry Stellenberg, Adam Preble

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions: 

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
