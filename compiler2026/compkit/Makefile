# Makefile for compkit
# Copyright (C) 1996-2005 Toyohashi University of Technology

SHELL	= /bin/sh

# set PREFIX
# example:
#	PREFIX = /home/ics-05/e050000
PREFIX	= $(HOME)/p1

#
BINDIR	= $(PREFIX)/bin
LIBDIR	= $(PREFIX)/lib
INCDIR	= $(PREFIX)/include
PERL	= '/usr/bin/env perl'

CC	= gcc
COPTS	= 

all: install

asm: asm.pl
	sed -e '1s!PERL!'$(PERL)'!' asm.pl > tmp-asm
	chmod +x tmp-asm
	mv -f tmp-asm asm

install: install-asm install-sr install-lib install-header

install-asm: asm
	-mkdir -p $(BINDIR)
	cp asm $(BINDIR)

install-sr: sr
	-mkdir -p $(BINDIR)
	bindir=`(cd $(BINDIR) ; pwd)` ; \
	(cd ./sr ; $(MAKE) install BINDIR=$$bindir)

install-lib: getsym.o
	-mkdir -p $(LIBDIR)
	ar rc $(LIBDIR)/libics.a getsym.o
	-if [ -f /usr/bin/ranlib -o -f /bin/ranlib ] ; then \
		(cd $(LIBDIR) ; ranlib libics.a) ; else true; fi

install-header: getsym.h
	-mkdir -p $(INCDIR)
	cp getsym.h $(INCDIR)

clean:
	(cd ./sr ; $(MAKE) clean)
	-rm -f *.o *~ asm

.c.o:
	$(CC) $(COPTS) -c $<
