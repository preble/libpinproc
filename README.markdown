## libpinproc

Library for [Multimorphic, Inc.'s](https://www.multimorphic.com/) P-ROC and P3-ROC pinball controller boards.

### Compiling

#### Prerequisites

libpinproc requires:

- [libusb](https://github.com/libusb/libusb): Install with the default /usr/local prefix.  Current builds of libftdi and libpinproc use libusb-1.0.

- [libftdi](https://www.intra2net.com/en/developer/libftdi/): Install with the default /usr/local prefix.

#### Building with CMake (Linux and macOS)

Download and install [CMake](https://cmake.org/download/).  Then:

    cd libpinproc
    mkdir bin; cd bin
    cmake ..
    make

The CMakeLists.txt file is presently designed to be run from a directory inside the libpinproc directory.  This will build both libpinproc and pinproctest.  Binaries will be placed in the directory that make was run from.  We recommend 'bin', as it is the path expected by pypinproc.

Note: On some systems, it may be necessary to build libpinproc with the '-fPIC' option.  To do this with cmake, instead of running 'cmake ..', run 'cmake .. -DCMAKE_CXX_FLAGS="-fPIC"'.  Compiling without '-fPIC' may cause problems when building the Python extensions on some 64-bit Linux machines.

Mac users may need to install [D2xxHelper](http://www.ftdichip.com/Drivers/D2XX.htm) to keep the Mac from claiming the P-ROC/P3-ROC and creating a `/dev/tty.usbserial` device for it.  It may be necessary to install a second time, if you're asked to allowing the installation via the Security & Privacy System Preference.  You will also need to reboot after installing D2xxHelper.  Run a `ls /dev/tty.usbserial*` before and after connecting the P-ROC/P3-ROC.  If you see a new entry, libpinproc will not be able to connect.

#### Building in Windows with MinGW/CMake

(Note that these instructions are outdated with the advent of of MSYS2/MinGW64.  Recent attempts at building with these instructions result in builds with dependencies on DLLs from MinGW64.  For users with MSYS/MinGW32 installations they should still be valid.)

Download and unzip [ftd2xx for Windows zip file](http://www.ftdichip.com/Drivers/D2XX.htm).  Plug in a powered-up P-ROC and point the driver install wizard to the unzipped driver.  Note, this is a two-step process.  It will ask you to install a driver twice (once for USB and again for the FTDI specific stuff).

Download and install [CMake](https://cmake.org/download/). 

Download and install [MinGW](http://sourceforge.net/projects/mingw/files/). (Tested with MinGW 5.1.4)

Follow directions above for Building with CMake with the following exception:
- Add `-G "MinGW Makefiles"` to the cmake command line.
- Use `-DEXTRA_INC="<path>"` and `-DEXTRA_LINK="<path>"` to add include/library paths for `ftd2xx.h` and `ftd2xx.sys`.
- Use mingw32-make instead of make.

#### Building in Windows with CMake and Visual Studio 2019

Follow directions above for Building with CMake with the following exception:
- Add `-A Win32` to the cmake command line.
- Use `-DEXTRA_INC="<path>"` and `-DEXTRA_LINK="<path>"` to add include/library paths for `ftd2xx.h` and `ftd2xx.sys`.
- Open PINPROC.sln in Visual Studio, switch to the Debug or Release configuration and perform ALL_BUILD.  It will place the libary and sample programs in `build/Debug` and `build/Release`.

Example:

    cd libpinproc
    mkdir bin; cd bin
    cmake .. -A Win32
    # configure paths for ftd2xxx; use `cmake .. -L` to list configured options
    cmake .. -D EXTRA_INC="../ftd2xx"
    cmake .. -D EXTRA_LINK="../ftd2xx/i386"
    # Can now open PINPROC.sln in Visual Studio and build Debug and Release targets.

### Testing

Once built, run the `pinproctest` program with the appropriate "machine type" passed in (e.g., "wpc").  Run `pinproctest` without any parameters for a list of valid types.

### License

Copyright (c) 2009 Gerry Stellenberg, Adam Preble

Copyright (c) 2020 Multimorphic, Inc.

Contributors:
  - Adam Preble
  - Gerry Stellenberg
  - Jan Kantert
  - Jimmy Lipham
  - Koen Heltzel
  - Roy Eltham
  - Tom Collins

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
