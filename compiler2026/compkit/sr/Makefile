# Makefile for sr
# Copyright (C) 1996-2005 Toyohashi University of Technology


CC	= gcc
OBJS	= sr.o proc.o loader.o
COPTS	= -O2

all: sr

sr: $(OBJS)
	$(CC) $(COPTS) -o sr $(OBJS) $(LIBS)

.c.o:
	$(CC) $(COPTS) -c $<

install: all
	-mkdir -p $(BINDIR)
	cp sr $(BINDIR)

clean:
	rm -f *.o *~ core a.out sr

sr.o: sr.c sr.h loader.h proc.h
proc.o: proc.c sr.h proc.h
loader.o: loader.c sr.h loader.h proc.h
