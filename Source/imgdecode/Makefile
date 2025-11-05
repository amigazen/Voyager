# Makefile for the Imgdecoders
#
# © 2000 VaporWare CVS team <ibcvs@vapor.com>
# All rights reserved
# $Id: Makefile,v 1.81 2003/06/22 21:48:58 zapek Exp $ 

CVSUSERS := owagner zapek sircus stuntzi kingguppy neko entity
CVSUSERNAME = $(strip $(foreach i, $(CVSUSERS), $(findstring $(i), $(shell grep .vapor.com:/home/cvs/cvsroot home:.cvspass))))

#########################################################################
#
# Work out what to build
#

# Chose between the way forward and outdated systems :)
ifeq ($(shell $(CC) -dumpmachine), ppc-morphos)
BUILD=MORPHOS
else
BUILD=AMIGAOS
endif


ifeq ($(BUILD),MORPHOS)
####
# MorphOS
#

PATH=/bin:/sc/c:/c

DELETE = rm
DELETEALL = rm -rf
TMPDIR = t:
SOURCE = ../../

BUILD=MORPHOS
PLATFORM=amiga
VPATH := ../../

LIBPOSTFIX=604e

endif


ifeq ($(BUILD),AMIGAOS)
####
# AmigaOS
#

PATH=/bin:/sc/c:/c

DELETE = rm
DELETEALL = rm -rf
TMPDIR = t:
SOURCE = //

BUILD=AMIGAOS
PLATFORM=amiga
VPATH := ../../

endif

BUILDDIR = objects


#########################################################################
#
# Global configs
#

# Those are the CVS users. You can add specific build scripts that will be
# run after the linking in the 'Post build procedure' section below

CVSUSERS := owagner zapek sircus stuntzi kingguppy


#########################################################################
#
# Objects
#
# This one has to be linked first
ifeq ($(BUILD),MORPHOS)
COBJS = noexec.o
endif

COBJS += adler32.o c2p_generic.o callbackfunctable.o crc32.o errorreq.o gif.o imgdecode.o infblock.o \
	infcodes.o inffast.o inflate.o inftrees.o infutil.o jcomapi.o jdapimin.o jdapistd.o jdcoefct.o jdcolor.o \
	jddctmgr.o jdhuff.o jdinput.o jdmainct.o jdmarker.o jdmaster.o jdmerge.o jdphuff.o jdpostct.o jdsample.o \
	jidctflt.o jidctfst.o jidctint.o jidctred.o jmemmgr.o jmemnobs.o jquant1.o jquant2.o jutils.o lib.o \
	libfunctable.o libfunctions.o mimebrushes.o png.o pngerror.o pngmem.o pngpread.o pngread.o pngrio.o pngrtran.o \
	pngrutil.o pngset.o pngtrans.o zutil.o

ASMOBJS := c2p.o saveds.o
AMIGAOBJS := executive.o


#########################################################################
#
# Programs
#
MAKE = make
BUMPER = rev

####
# AmigaOS
#
ifeq ($(BUILD),AMIGAOS)

PATH=/bin:/sc/c:/c

DEBUG=

HOSTCC = sc
CROSSCC = sc
HOSTAS = sc
CROSSAS = sc
HOSTLINK = slink
CROSSLING = slink

OBJS = $(COBJS) $(ASMOBJS) $(AMIGAOBJS)

CPU-TYPES = 68020 68030 68040 68060

# Find out the CPU options
ifeq ($(CPU), 68020)
CCOPTS +=
MATH = IEEE
LIB = lib:scmieee.lib
LIBPOSTFIX = 68020
endif

ifeq ($(CPU), 68030)
CCOPTS +=
MATH = 68881
LIB = lib:scm881.lib
LIBPOSTFIX = 68030fpu
endif

ifeq ($(CPU), 68040)
CCOPTS +=
MATH = 68881
LIB = lib:scm040.lib
LIBPOSTFIX = 68040fpu
endif

ifeq ($(CPU), 68060)
CCOPTS +=
MATH = 68881
LIB = lib:scm040.lib
LIBPOSTFIX = 68060
endif

LIB +=,lib:sc.lib,lib:debug.lib,lib:small.lib FROM lib:libent.o,lib:libinit.o

CCOPTS := DEF=AMIGAOS OBJECTNAME="" DEF=NO_GETENV CPU=$(CPU) MATH=$(MATH)
LDOPTS = LIBFD $(SOURCE)fd/vimgdecode_lib.fd SC SD NOICONS ADDSYM LIBVERSION `cat env:libver` LIBREVISION `cat env:librev` LIBID "`cat env:libid`" WITH LIB:utillib.with

# For the Amiga we find out if there's a need to run postbuild commands
CVSUSERNAME = $(strip $(foreach i, $(CVSUSERS), $(findstring $(i), $(shell grep @water.vapor.com:/home/cvs/cvsroot home:.cvspass))))

endif

ifeq ($(BUILD),MORPHOS)
####
# MorphOS
#

HOSTCC = gcc
CROSSCC = gcc
HOSTAS = gcc
CROSSAS = gcc
HOSTLINK = gcc
CROSSLINK = gcc

OBJS = $(COBJS)

#DEBUG=-DVDEBUG=1
DEBUG=

CPU-TYPES = 604e

CCOPTS = -g -noixemul -nostdlib -O2 -mcpu=$(CPU) -mmultiple -fomit-frame-pointer -DUSE_INLINE_STDARG \
	-Wall -Wno-parentheses -Wno-switch -Wstrict-prototypes -Werror-implicit-function-declaration \
	-Wmissing-prototypes -Wmissing-declarations \
	-I$(SOURCE). -I$(SOURCE).. -I$(SOURCE)../.. -I$(SOURCE)../../include
LDOPTS = -o

endif #MorphOS


#########################################################################
#
# Executables and objects for installation
#


#########################################################################
#
# Targets
#
.PHONY: build all debug clean mrproper

build:
	@echo ""
	@echo "Imgdecoder build in progress... MOTD: plop"
	@echo ""
	@-for i in $(CPU-TYPES); \
		do (echo ""; \
			echo -n "Making "; \
			echo -n $$i; \
			echo " version ..."; \
			echo ""; \
			mkdir -p $(BUILDDIR)/$$i; \
			cd $(BUILDDIR)/$$i; \
			cp ../../SCOPTIONS SCOPTIONS; \
			$(MAKE) -f ../../Makefile all CPU=$$i ) ; done

ifeq ($(BUILD),AMIGAOS)
vimgdec_$(LIBPOSTFIX).vlib: $(OBJS)
	$(BUMPER) DIR=$(SOURCE) LIBID vimgdec_$(LIBPOSTFIX) GLOBAL
	slink LIB $(LIB) $(LDOPTS) from $(OBJS) TO vimgdec_$(LIBPOSTFIX).vlib
	-@delete vimgdec_$(LIBPOSTFIX).vlib.db
	rename vimgdec_$(LIBPOSTFIX).vlib vimgdec_$(LIBPOSTFIX).vlib.db
	slink vimgdec_$(LIBPOSTFIX).vlib.db to vimgdec_$(LIBPOSTFIX).vlib STRIPDEBUG NOICONS
endif
ifeq ($(BUILD),MORPHOS)
vimgdec_$(LIBPOSTFIX).vlib: $(OBJS)
	ld -fl libnix $(OBJS) -labox -lstringio -lmath -lmoto -lsyscall -lc -o vimgdec_604e.vlib.db
	strip --remove-section=.comment -o vimgdec_604e.vlib vimgdec_604e.vlib.db
endif

all: vimgdec_$(LIBPOSTFIX).vlib postbuild_$(CVSUSERNAME)

# use the DB macro to output debuging stuffs
debug:
	$(MAKE) DEBUG=DEF=DEBUG=2

update:
	$(BUMPER) INCREV

clean:
	-$(DELETEALL) $(BUILDDIR)

mrproper: clean

ppccheck:
	objdump --section-headers --all-headers --reloc --disassemble-all objects/604e/vimgdec_604e.vlib | grep UND | grep -v PPC

dump:
	ppc-amigaos-objdump --section-headers --all-headers --reloc --disassemble-all --syms objects/604e/vimgdec_604e.vlib.db >objects/604e/vimgdec_604e.vlib.dump

68k:
	$(MAKE) $f Makefile build BUILD=AMIGAOS

#########################################################################
#
# Global rules
#
.SUFFIXES: .o$(TARGET) .asm

ifeq ($(BUILD),MORPHOS)
.c.o$(TARGET):
	$(CROSSCC) $(CCOPTS) $(DEBUG) -c $(SOURCE)$*.c
endif

ifeq ($(BUILD),AMIGAOS)
.c.o$(TARGET):
	$(CROSSCC) $(CCOPTS)  $(DEBUG) $(SOURCE)$*.c
endif




#.asm.o$(TARGET):
#    basm $(AFLAGS) $(APATH) -o$*.o$(TARGET) $*.asm


#########################################################################
#
# Rules
#
adler32.o$(TARGET): adler32.c zlib.h zconf.h

crc32.o$(TARGET): crc32.c zlib.h zconf.h

gif.o$(TARGET): gif.c gif.h gifdecode.h

imgdecode.o$(TARGET): imgdecode.c ../classes.h ../imgcallback.h rev.h c2p.h

infblock.o$(TARGET): infblock.c zutil.h zlib.h zconf.h infblock.h inftrees.h infcodes.h infutil.h

infcodes.o$(TARGET): infcodes.c zutil.h zlib.h zconf.h inftrees.h infblock.h infcodes.h infutil.h inffast.h

inffast.o$(TARGET): inffast.c zutil.h zlib.h zconf.h inftrees.h infblock.h infcodes.h infutil.h inffast.h

inflate.o$(TARGET): inflate.c zutil.h zlib.h zconf.h infblock.h

inftrees.o$(TARGET): inftrees.c zutil.h zlib.h zconf.h inftrees.h inffixed.h

infutil.o$(TARGET): infutil.c zutil.h zlib.h zconf.h infblock.h inftrees.h infcodes.h infutil.h

jcomapi.o$(TARGET): jcomapi.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h

jdapimin.o$(TARGET): jdapimin.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h

jdapistd.o$(TARGET): jdapistd.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h

jdcoefct.o$(TARGET): jdcoefct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h

jdcolor.o$(TARGET): jdcolor.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h

jddctmgr.o$(TARGET): jddctmgr.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h

jdhuff.o$(TARGET): jdhuff.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdhuff.h

jdinput.o$(TARGET): jdinput.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h

jdmainct.o$(TARGET): jdmainct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h

jdmarker.o$(TARGET): jdmarker.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h

jdmaster.o$(TARGET): jdmaster.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h

jdmerge.o$(TARGET): jdmerge.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h

jdphuff.o$(TARGET): jdphuff.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdhuff.h

jdpostct.o$(TARGET): jdpostct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h

jdsample.o$(TARGET): jdsample.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h

jidctflt.o$(TARGET): jidctflt.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h

jidctfst.o$(TARGET): jidctfst.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h

jidctint.o$(TARGET): jidctint.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h

jidctred.o$(TARGET): jidctred.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h

jmemmgr.o$(TARGET): jmemmgr.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jmemsys.h

jmemnobs.o$(TARGET): jmemnobs.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jmemsys.h

jquant1.o$(TARGET): jquant1.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h

jquant2.o$(TARGET): jquant2.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h

jutils.o$(TARGET): jutils.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h

mimebrushes.o$(TARGET): mimebrushes.c

png.o$(TARGET): png.c png.h zlib.h zconf.h pngconf.h

pngerror.o$(TARGET): pngerror.c png.h zlib.h zconf.h pngconf.h

pngmem.o$(TARGET): pngmem.c png.h zlib.h zconf.h pngconf.h

pngpread.o$(TARGET): pngpread.c png.h zlib.h zconf.h pngconf.h

pngread.o$(TARGET): pngread.c png.h zlib.h zconf.h pngconf.h

pngrio.o$(TARGET): pngrio.c png.h zlib.h zconf.h pngconf.h

pngrtran.o$(TARGET): pngrtran.c png.h zlib.h zconf.h pngconf.h

pngrutil.o$(TARGET): pngrutil.c png.h zlib.h zconf.h pngconf.h

pngset.o$(TARGET): pngset.c png.h zlib.h zconf.h pngconf.h

pngtrans.o$(TARGET): pngtrans.c png.h zlib.h zconf.h pngconf.h

zutil.o$(TARGET): zutil.c zutil.h zlib.h zconf.h


#########################################################################
#
# MorphOS stuffs
#
lib.o$(TARGET): lib.c rev.h

libfunctable.o$(TARGET): libfunctable.c

libfunctions.o$(TARGET): libfunctions.c

c2p_generic.o$(TARGET): c2p_generic.c c2p.h

callbackfunctable.o$(TARGET): callbackfunctable.c

#########################################################################
#
# ASM stuffs (SAS/C's asm really sucks so we use sc)
#
ifeq ($(BUILD), AMIGAOS)
c2p.o: c2p.s
	$(CROSSAS) CPU=$(CPU) $(SOURCE)c2p.s
	mv ../../c2p.o .

saveds.o: c2p.s
	$(CROSSAS) CPU=$(CPU) $(SOURCE)saveds.s
	mv ../../saveds.o .
endif


#########################################################################
#
# Amiga specific C files
#
executive.o: ../executive.c
	$(CROSSCC) $(CCOPTS) INCDIR=//// $(DEBUG) $(SOURCE)/executive.c


#########################################################################
#
# Misc stuffs, used by MorphOS only atm
#
rev.h: .revinfo
	$(BUMPER) DIRECTORY=//
	cp ../../rev.h .


########################################################################
#
# Post build procedure - user settable
#

# Dictator Olli
postbuild_owagner:
	@echo -n "Installing "
	@echo -n $(CPU)
	@echo " version..."
	@echo ""
	@mkdir -p ../../../Plugins
	@copy vimgdec_$(LIBPOSTFIX).vlib to ///Plugins/vimgdec_$(LIBPOSTFIX).vlib

# muiman stuntzi
postbuild_stuntzi:
	@echo -n "Installing "
	@echo -n $(CPU)
	@echo " version..."
	@echo ""
	@mkdir -p ../../../Plugins
	@copy vimgdec_$(LIBPOSTFIX).vlib to ///Plugins/vimgdec_$(LIBPOSTFIX).vlib


# Slacker KingGuppy
postbuild_kingguppy:
	@echo -n "Installing "
	@echo -n $(CPU)
	@echo " version..."
	@echo ""
	@mkdir -p Internet:WWW/Voyager/Plugins
	@copy vimgdec_$(LIBPOSTFIX).vlib to Internet:WWW/Voyager/Plugins/vimgdec_$(LIBPOSTFIX).vlib

# Master Zapek
postbuild_zapek:
	@echo -n "Installing "
	@echo -n $(CPU)
	@echo " version..."
	@echo ""
	@mkdir -p ../../../Plugins
	@copy vimgdec_$(LIBPOSTFIX).vlib to amitcp:/voyager/plugins
	-flushlib vimgdec_$(LIBPOSTFIX).vlib

# Default global rule
postbuild_:
	@echo "No specific postbuild target to be done"
