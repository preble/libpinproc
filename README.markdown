## libpinproc

Library for [Multimorphic, Inc.'s](https://www.multimorphic.com/) P-ROC and P3-ROC pinball controller boards.

### Compiling

#### Prerequisites

libpinproc requires:

- [libusb-0.1.12](http://libusb.wiki.sourceforge.net/): Install with the default /usr/local prefix.  Version 0.1.12 has been tested on Mac and Linux.  Mac users: If you want to use libpinproc under Cocoa or pygame, you may wish to try libusb 1.0.  See below.

- [libftdi-0.16](http://www.intra2net.com/en/developer/libftdi/): Install with the default /usr/local prefix.

##### libusb-1.0 and libusb-compat

Version 1.0.2 does not work out of the box since libftdi is written against libusb-0.1.  You can use the libusb-compat-0.1.2 project, however, which creates a library that provides the older libusb interface.  Because Macs do not come with pkg-config, you may need to run configure for libusb-compat as follows:

    ./configure LIBUSB_1_0_CFLAGS=-I/usr/local/include/libusb-1.0 LIBUSB_1_0_LIBS="-L/usr/local/lib -lusb-1.0"

Note that libusb-1.0 must have been built and installed prior to this step.  This also assumes that you installed libusb-1.0 with the default /usr/local prefix.

#### Building with CMake

Download and install [CMake](http://www.cmake.org/cmake/resources/software.html).  Then:

    cd libpinproc
    mkdir bin; cd bin
    cmake ..
    make

The CMakeLists.txt file is presently designed to be run from a directory inside the libpinproc directory.  This will build both libpinproc and pinproctest.  Binaries will be placed in the directory that make was run from.  We recommend 'bin', as it is the path expected by pypinproc.

Note: On some systems, it may be necessary to build libpinproc with the '-fPIC' option.  To do this with cmake, instead of running 'cmake ..', run 'cmake .. -DCMAKE_CXX_FLAGS="-fPIC"'.  Compiling without '-fPIC' may cause problems when building the Python extensions on some 64-bit Linux machines.

#### Building in Windows with MinGW/CMake

Download and unzip [ftd2xx for Windows zip file](http://www.ftdichip.com/Drivers/D2XX.htm).  Plug in a powered-up P-ROC and point the driver install wizard to the unzipped driver.  Note, this is a two-step process.  It will ask you to install a driver twice (once for USB and again for the FTDI specific stuff).

Download and install [CMake](http://www.cmake.org/cmake/resources/software.html). 

Download and install [MinGW](http://sourceforge.net/projects/mingw/files/). (Tested with MinGW 5.1.4)

Follow directions above for building yaml-cpp with the following exception:
 add '-G "MinGW Makefiles"' to the cmake command line.

To build libpinproc:

- Use -DEXTRA_INC="<path>;<path>" and -DEXTRA_LINK="<path>;<path>" to add include/library paths for `ftd2xx.h` and `ftd2xx.sys`.

Follow instructions above for building libpinproc with cmake with the following exceptions:
 add '-G "MinGW Makefiles' to the cmake command line,
 use mingw32-make instead of make

#### Building in Windows with Visual Studio 2019

Assumes you are already running an MSYS2/MinGW64 system.

Make sure you have cmake installed for i686 (MinGW32) or x86_64 (MinGW64): `pacman -S cmake mingw-w64-i686-cmake mingw-w64-x86_64-cmake`

    cd libpinproc
    mkdir build; cd build
    cmake .. -A Win32
    # configure paths for td2xxx; use `cmake .. -L` to list configured options
    cmake .. -D EXTRA_INC="../ftd2xx"
    cmake .. -D EXTRA_LINK="../../ftd2xx/i386"

Then open PINPROC.sln in Visual Studio, switch to the Debug or Release configuration and perform ALL_BUILD.  It will place the libary and sample programs in `build/Debug` and `build/Release`.

### License

Copyright (c) 2009 Gerry Stellenberg, Adam Preble
Copyright (c) 2020 Multimorphic, Inc.

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
