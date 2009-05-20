#
# File:  Makefile for application
#
CC=g++
LDFLAGS=-L../.. -L/usr/local/lib
LIBS=-lpinproc -lusb -lftdi

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
