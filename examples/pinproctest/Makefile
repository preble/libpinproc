#
# File:  Makefile for application
#
CC=g++
LDFLAGS=-L../.. -L/usr/local/lib -L../../../../yaml-cpp/build/bin
LIBS=-lpinproc -lusb -lftdi -lyaml-cpp

SRC=pinproctest.cpp

CXXFLAGS=-I../../include

OBJS=$(SRC:.cpp=.o)

EXE=pinproctest

all: $(EXE)

# FIXME: This makes the exe require libpinproc but not in a very graceful way.
$(EXE): $(OBJS) ../../libpinproc.a
	@echo application Makefile - linking $<
	$(CC) $^ $(LDFLAGS) $(LIBS) -o $@

.cpp.o:
	@echo application Makefile - compiling $<
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXE)
