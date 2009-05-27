## libpinproc

Library for Gerry Stellenberg's [P-ROC](http://pinballcontrollers.com/) (Pinball Remote Operations Controller).

### Compiling

#### Prerequisites

libpinproc requires:

- [libusb-0.1.12](http://libusb.wiki.sourceforge.net/): Install with the default /usr/local prefix.
- [libftdi-0.16](http://www.intra2net.com/en/developer/libftdi/): Install with the default /usr/local prefix.

The pinproctest example requires [yaml-cpp](http://code.google.com/p/yaml-cpp/). Follow the build instructions, creating the build subdirectory.  After building, from the build directory, run the following commands to manually install it:

    sudo cp bin/libyaml-cpp.a /usr/local/lib/
    sudo mkdir /usr/local/include/yaml-cpp
    sudo cp ../include/*.h /usr/local/include/yaml-cpp/

#### Building with CMake

Download and install [CMake](http://www.cmake.org/cmake/resources/software.html).  Then:

    cd libpinproc
    mkdir build; cd build
    cmake ..
    make

The CMakeLists.txt file is presently designed to be run from a directory inside the libpinproc directory.  This will build both libpinproc and pinproctest.  Binaries will be placed in the directory that make was run from.

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
