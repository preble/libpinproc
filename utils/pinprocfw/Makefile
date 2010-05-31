CC = g++
RM = rm -f
CFLAGS = $(ARCH) -c -Wall -I../../include
LDFLAGS = $(ARCH) -L../../bin

uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

PINPROCFW = ../../bin/pinprocfw
LIBPINPROC = ../../bin/libpinproc.a
SRCS = pinprocfw.cpp lenval.cpp
OBJS := $(SRCS:.cpp=.o)
INCLUDES = ../../include/pinproc.h

LIBS = usb pinproc
ifneq ($(uname_s),Windows) # not Windows
	LIBS += ftdi
endif
ifeq ($(uname_s),Windows)
	LIBS = ftd2xx
endif

pinprocfw: $(PINPROCFW)

$(PINPROCFW): $(OBJS) $(LIBPINPROC)
	$(CC) $(LDFLAGS) $(OBJS) $(addprefix -l,$(LIBS)) -o $@

.cpp.o:
	$(CC) $(CFLAGS) -o $@ $<

clean:
	$(RM) $(OBJS)

.PHONY: clean pinprocfw

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it

pinprocfw.o: lenval.h ../../include/pinproc.h
lenval.o: lenval.h pinprocfw.h
