# Makefile for pcf2bdf (gcc)  -*- makefile -*-

CC			= g++
CFLAGS		= -Wall -O2
CXX			= $(CC)
CXXFLAGS	= $(CFLAGS)
PREFIX		= /usr/local
BINPATH		= $(PREFIX)/bin
MANPATH		= $(PREFIX)/man/man1
all:		pcf2bdf

pcf2bdf:	pcf2bdf.o

clean:
	-$(RM) pcf2bdf pcf2bdf.exe pcf2bdf.o *~

install:	all
	-@if [ ! -d $(BINPATH)  ]; then mkdir -p $(BINPATH); fi
	install -m 755 pcf2bdf $(BINPATH)
	-@if [ ! -d $(MANPATH)  ]; then mkdir -p $(MANPATH); fi
	install -m 644 pcf2bdf.man $(MANPATH)/pcf2bdf.1
