CC = g++
AR = ar
ARFLAGS = rc
RANLIB = ranlib
RM = rm -f
LIBPINPROC_CFLAGS=-c -Wall -Iinclude

LIBPINPROC = bin/libpinproc.a
LIBPINPROC_DYLIB = bin/libpinproc.dylib
SRCS = src/pinproc.cpp src/PRDevice.cpp src/PRHardware.cpp
OBJS := $(SRCS:.cpp=.o)
INCLUDES = include/pinproc.h src/PRCommon.h src/PRDevice.h src/PRHardware.h
LIBS = usb ftdi

.PHONY: libpinproc
libpinproc: $(LIBPINPROC) $(LIBPINPROC_DYLIB)

$(LIBPINPROC): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)
	$(RANLIB) $@

$(LIBPINPROC_DYLIB): $(OBJS)
	g++ -dynamiclib -o $@ /usr/local/lib/libftdi.dylib $(LDFLAGS) $(OBJS)

.cpp.o:
	$(CC) $(LIBPINPROC_CFLAGS) $(CFLAGS) -o $@ $<

clean:
	$(RM) $(OBJS)

.PHONY: clean

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it

src/PRDevice.o: include/pinproc.h src/PRCommon.h src/PRHardware.h
src/PRHardware.o: include/pinproc.h
src/pinproc.o: include/pinproc.h src/PRDevice.h
src/pinproc.o: src/PRCommon.h src/PRHardware.h
src/PRDevice.o: src/PRDevice.h include/pinproc.h
src/PRDevice.o: src/PRCommon.h src/PRHardware.h
src/PRHardware.o: src/PRHardware.h include/pinproc.h
src/PRHardware.o: src/PRCommon.h
