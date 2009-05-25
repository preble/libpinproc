## libpinproc

Library for Gerry Stellenberg's [P-ROC](http://pinballcontrollers.com/) (Pinball Remote Operations Controller).

### Compiling

#### Prerequisites

libpinproc requires:

- [libusb-0.1.12](http://libusb.wiki.sourceforge.net/): Install with the default /usr/local prefix.
- [libftdi-0.16](http://www.intra2net.com/en/developer/libftdi/): Install with the default /usr/local prefix.

Once required but not right now:

- [yaml-cpp](http://code.google.com/p/yaml-cpp/): Should be checked out in the directory two levels below libpinproc in the full source tree (at the same level as ./P-ROC) in a directory named yaml-cpp.  Follow the build instructions, creating the build subdirectory.  The Makefiles and other project files expect to find libyaml-cpp.a in the yaml-cpp/build/bin directory.

We are presently experimenting with different build mechanisms.  As such there are three ways to build libpinproc at this time: CMake, GNU Make, and the Xcode project (for Mac).  As the preferred method is presently CMake, here's how.  Before you get started, you will need CMake if you don't already have it.

#### Building with CMake

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
