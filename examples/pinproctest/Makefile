#
# File:  Makefile for application
#
PINPROC_PATH=../..

CC=g++
LDFLAGS=-L$(PINPROC_PATH)/bin -L/usr/local/lib
LIBS=-lpinproc -lusb -lftdi

SRC=pinproctest.cpp

CXXFLAGS=-I$(PINPROC_PATH)/include

OBJS=$(SRC:.cpp=.o)

EXE=pinproctest

all: $(EXE)

# FIXME: This makes the exe require libpinproc but not in a very graceful way.
$(EXE): $(OBJS) $(PINPROC_PATH)/bin/libpinproc.a
	@echo application Makefile - linking $<
	$(CC) $^ $(LDFLAGS) $(LIBS) -o $@

.cpp.o:
	@echo application Makefile - compiling $<
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXE)
