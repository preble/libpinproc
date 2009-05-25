#
# File:  Makefile (for library)
#
CC=g++
LIB=libpinproc.a
LIBDEST=./bin/

LIBSRC=src/pinproc.cpp src/PRDevice.cpp src/PRHardware.cpp

LIBOBJ=$(LIBSRC:.cpp=.o)

CXXFLAGS=

$(LIB): $(LIBOBJ)
	@echo lib Makefile - archiving $(LIB)
	$(AR) r $(LIBDEST)$(LIB) $(LIBOBJ)

.cpp.o:
	@echo lib Makefile - compiling $<
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(LIBOBJ) $(LIBDEST)$(LIB)
