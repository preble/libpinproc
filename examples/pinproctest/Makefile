CC = g++
RM = rm -f
CFLAGS = $(ARCH) -c -Wall -I../../include
LDFLAGS = $(ARCH) -L../../bin

uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

PINPROCTEST = ../../bin/pinproctest
LIBPINPROC = ../../bin/libpinproc.a
SRCS = pinproctest.cpp drivers.cpp dmd.cpp switches.cpp
OBJS := $(SRCS:.cpp=.o)
INCLUDES = ../../include/pinproc.h

LIBS = usb pinproc
ifneq ($(uname_s),Windows) # not Windows
	LIBS += ftdi
endif
ifeq ($(uname_s),Windows)
	LIBS = ftd2xx
endif

pinproctest: $(PINPROCTEST)

$(PINPROCTEST): $(OBJS) $(LIBPINPROC)
	$(CC) $(LDFLAGS) $(OBJS) $(addprefix -l,$(LIBS)) -o $@

.cpp.o:
	$(CC) $(CFLAGS) -o $@ $<

clean:
	$(RM) $(OBJS)

.PHONY: clean pinproctest

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it

pinproctest.o: pinproctest.h ../../include/pinproc.h
drivers.o: pinproctest.h ../../include/pinproc.h
dmd.o: pinproctest.h ../../include/pinproc.h
switches.o: pinproctest.h ../../include/pinproc.h
