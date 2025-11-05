# Makefile for VFlash.module
# ©1999 by David Gerber <zapek@vapor.com>
# All rights reserved
#
# $Id: Makefile,v 1.6 2004/02/28 18:51:07 zapek Exp $
#
# Tip of the day:
#
# To produce a libdebug.a:
# hunk2aout debug.lib
# ar qc libdebug.a obj.*
# ranlib libdebug.a

VPATH := $(SOURCE)module

FLASH = $(SOURCE)Lib
PLUGIN = $(SOURCE)plugin

PLATFORM = amiga

include $(SOURCE)module/makefile.objs

# With libnix
CFLAGS += -I$(FLASH) -I$(PLUGIN) -I$(PLUGIN)/include -I../..
LIBS = libjpeg.a libz.a
MLIBS = -labox -lmoto -lmath -laboxstubs -lc -lstringio -lsyscall
ALIBS = -lamiga -lm #-ldebug

VFlash_debug.module: $(OBJS) libflash.o ../rev.h
	$(CC) $(CFLAGS) -noixemul -o $@ $(OBJS) $(SWFOBJS) $(LIBS) $(ALIBS)

VFlash_68020.module: $(OBJS) libflash.o ../rev.h
	$(CC) $(CFLAGS) -noixemul -o $@.debug $(OBJS) $(SWFOBJS) $(LIBS) $(ALIBS)
	strip $@.debug -o $@

VFlash_68030fpu.module: $(OBJS) libflash.o ../rev.h
	$(CC) $(CFLAGS) -noixemul -o $@.debug $(OBJS) $(SWFOBJS) $(LIBS) $(ALIBS)
	strip $@.debug -o $@

VFlash_68040fpu.module: $(OBJS) libflash.o ../rev.h
	$(CC) $(CFLAGS) -noixemul -o $@.debug $(OBJS) $(SWFOBJS) $(LIBS) $(ALIBS)
	strip $@.debug -o $@

VFlash_68060.module: $(OBJS) libflash.o ../rev.h
	$(CC) $(CFLAGS) -noixemul -o $@.debug $(OBJS) $(SWFOBJS) $(LIBS) $(ALIBS)
	strip $@.debug -o $@

VFlash_ppc.module: $(OBJS) libflash.o ../rev.h
	$(CC) $(CFLAGS) -r -o VFlash_ppcdebug.module $(OBJS) $(SWFOBJS) $(LIBS)
# sigh, -s doesn't work at all since the result is an object file in fact (that's the way ppc.library works)
	ppc-amigaos-egcs-objcopy --strip-unneeded --remove-section=.comment --strip-debug --discard-all --discard-locals -v VFlash_ppcdebug.module VFlash_ppc.module
	protect VFlash_ppc.module +e

VFlash_m604e.module: $(OBJS) libflash.o ../rev.h
	$(CC) $(CFLAGS) -noixemul -o $@.debug $(OBJS) $(SWFOBJS) $(LIBS) $(MLIBS)
	ppc-morphos-strip --remove-section=.comment $@.debug -o $@
clean:
	rm -f *.o

