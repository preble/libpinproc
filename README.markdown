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

Note: On some systems, it may be necessary to build libpinproc with the '-fPIC' option.  To do this with cmake, instead of running 'cmake ..', run 'cmake .. -DCMAKE_CXX_FLAGS="-fPIC"'.  Compiling without '-fPIC' may cause problems when building the python extensions on some 64-bit Linux machines.

#### Building in Windows with MinGW/CMake

Download and unzip [ftd2xx for Windows zip file](http://www.ftdichip.com/Drivers/D2XX.htm).  Plug in a powered-up P-ROC and point the driver install wizard to the unzipped driver.  Note, this is a two-step process.  It will ask you to install a driver twice (once for USB and again for the FTDI specific stuff).

Download and install [CMake](http://www.cmake.org/cmake/resources/software.html). 

Download and install [MinGW](http://sourceforge.net/projects/mingw/files/). (Tested with MinGW 5.1.4)

Follow directions above for building yaml-cpp with the following exception:
 add '-G "MinGW Makefiles"' to the cmake command line.

To build libpinproc:
 add the paths for ftd2xx.h (from the unzipped driver package) and the yaml-cpp/include/*.h files to the "include_directories" line in libpinproc/CMakeLists.txt

Until an automatic build process/structure is worked out:
 change libpinproc/examples/pinproctest/pinproctest.h to look for yaml.h locally: "yaml.h"

Follow instructions above for building libpinproc with cmake with the following exceptions:
 add '-G "MinGW Makefiles' to the cmake command line,
 use mingw32-make instead of make
 
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
