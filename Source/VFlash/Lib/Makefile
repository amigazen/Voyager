# Makefile for the Flash Library
#
# $Id: Makefile,v 1.3 2004/02/28 18:51:06 zapek Exp $
#

srcdir = .

ifeq ($(srcdir),.)
srcdir = ../../Lib
endif

VPATH := $(srcdir)

CFLAGS += -I../.. -I../../plugin -I../../plugin/include
CPPFLAGS = $(CFLAGS)

INCLUDES = \
	adpcm.h \
	cxform.h \
	graphic.h \
	script.h \
	bitmap.h \
	displaylist.h \
	matrix.h \
	shape.h \
	swf.h \
	button.h \
	flash.h \
	program.h \
	sound.h \
	text.h \
	character.h \
	font.h \
	rect.h \
	sprite.h \
	movie.h

SWFOBJS = \
	amiga.o \
	flash.o \
	character.o \
	shape.o \
	button.o \
	program.o \
	bitmap.o \
	displaylist.o \
	font.o \
	graphic.o \
	text.o \
	matrix.o \
	script.o \
	sound.o \
	sprite.o \
	movie.o \
	cxform.o \
	adpcm.o


all: libflash.o

# this needs fixing as libjpeg.a or libz.a aren't rebuild when needed :|
libflash.o: $(SWFOBJS) libjpeg.a libz.a
	$(CC) $(LDFLAGS) -r -o $@ $(SWFOBJS) libjpeg.a libz.a

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o : %.cc
	$(CC) $(CFLAGS) -c $< -o $@

$(SWFOBJS): $(INCLUDES)

libjpeg.a:
	$(MAKE) -f ../../Lib/Jpeg/Makefile

libz.a:
	$(MAKE) -f ../../Lib/Zlib/Makefile

clean:
	-rm -f $(SWFOBJS) libflash.o
	$(MAKE) -f ../../Lib/Jpeg/Makefile clean
	$(MAKE) -f ../../Lib/Zlib/Makefile clean

.PHONY: plugin jpeg all
# DO NOT DELETE
