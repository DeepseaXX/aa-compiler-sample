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
LIBS	= -lics
OPTS	= -O2 -Wall -I. -I$(INCDIR) -L$(LIBDIR) $(LIBS)
OBJS	= main.o parse.o

ASM	= $(BINDIR)/asm
SR	= $(BINDIR)/sr


target: compiler


compiler: $(OBJS)	
	$(CC) $(OBJS) -o compiler $(OPTS)

main.o: main.c
	$(CC) main.c -c $(OPTS)
parse.o: parse.c
	$(CC) parse.c -c $(OPTS)

asm:
	$(ASM) a.asm > a.out

sr:
	$(SR) a.out

clean:
	-rm -f *.o *~ compiler
