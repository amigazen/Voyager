# Makefile for Voyager
#
# © 1999-2001 VaporWare CVS team <ibcvs@vapor.com>
# All rights reserved
#
# $Id: makefile,v 1.150 2004/11/16 16:01:15 zapek Exp $
#

# Targets:
#
# make: full V build
# make depend: generate V dependencies
# make clean: cleans up the whole objects directory
# make mrproper: cleans better (executables, bison output, etc...)
# make alpha: creates an alpha version + archives it to ram:. a V.debug.<date> is automatically created
# make debug: version with forced debug output
# make update: increments the compilation/build n (x.x.n)
# make 68k: for a 68k version when one is under MorphOS
# make loader: to create the loader module

#########################################################################
#
# Global configs
#

# Those are the CVS users. You can add specific build scripts that will be
# run after the linking in the 'Post build procedure' section below

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

BUILDDIR = objects

include makefile.objs


#########################################################################
#
# Programs
#
BUMPER = rev
MAKEFILENAME = makefile


ifeq ($(BUILD),AMIGAOS)
###################################
#
# AmigaOS
#

PATH=/bin:/sc/c:/c

MAKE = make
VDEBUG=

DELETE = rm -f
DELETEALL = rm -rf
TMPDIR = t:
SOURCE = //

PLATFORM=amiga
VPATH := ../../
ARCH=680?0


NATVCC = sc
CROSSCC = sc
HOSTAS = asm
CROSSAS = asm
HOSTLINK = slink
CROSSLINK = slink
SCOPTS = ../../SCOPTIONS
CATMAKER = catmaker

OBJS = $(COBJS) $(COBJS_NODEP) $(ASMOBJS) $(PLATFORMOBJS) $(AMIGAOBJS)

CPU-TYPES = 68020

# Includes are also in SCOPTIONS for SAS/C
INCLUDES = -I/include -I. -I.. -I../vat -I../include -I../libmath64 -I/netinclude/ -Iimgdecode -Ipluginapi -Iaboutstuff
DEPINCLUDES = $(INCLUDES)

# Since every serious Amiga programmer runs under
# MorphOS ;) we undefine PPC symbols to avoid troubles
# during the generation of dependencies for those still running
# under old and outdated 680x0 CPUs
DEPOPTS = -DDEPEND -DVDEBUG -U__PPC__ -U__MORPHOS__ -U__powerpc__ -DAMIGAOS
DEPFILE =.depend.amigaos

CCOPTS = DEF=AMIGAOS OBJECTNAME="" CODE=FAR
LCCOPTS = CODE=FAR IGN=72
LDOPTS = 
HOSTCCOPTS = DEF=AMIGAOS

endif # AmigaOS

ifeq ($(BUILD),MORPHOS)
###################################
#
# MorphOS
#

PATH=/bin:/sc/c:/sys/c_ppc:/c

MAKE = make
VDEBUG=

DELETE = rm -f
DELETEALL = rm -rf
TMPDIR = /tmp/
SOURCE = ../../

BUILD=MORPHOS
PLATFORM=morphos
VPATH := $(SOURCE)
ARCH=60?e

NATVCC = gcc
CROSSCC = ppc-morphos-gcc
HOSTAS = as
CROSSAS = ppc-morphos-as
HOSTLINK = ld
CROSSLINK = ppc-morphos-gcc
CATMAKER = catmaker_mbx

OBJS = $(COBJS) $(COBJS_NODEP) $(GENERICOBJS) $(PLATFORMOBJS) $(AMIGAOBJS)

CPU-TYPES = 604e

INCLUDES = -I- -I$(SOURCE). -I$(SOURCE).. -I$(SOURCE)../libmath64 -I$(SOURCE)includestuff \
	-I$(SOURCE)../include
DEPINCLUDES = -I. -I.. -I../libmath64 -Iincludestuff \
	-I../include

DEPOPTS = -DDEPEND -DVDEBUG
DEPFILE =.depend.morphos

CCOPTS = -c -noixemul -O2 -DSYSTEM_PRIVATE -fomit-frame-pointer -mmultiple -mcpu=$(CPU-TYPES) \
	-Wformat -Wunused -Wuninitialized -Wstrict-prototypes -Wpointer-arith \
	-Werror-implicit-function-declaration $(INCLUDES)
LDOPTS = -noixemul -o
HOSTCCOPTS = -c -fomit-frame-pointer -DAMIGAOS

# 3rd party include directory for MorphOS
#INCLUDE3RD = /gg/morphos/emulinclude/include3rd/

# MorphOS includes
#MOSINCLUDE = /gg/morphos/emulinclude/

endif # MorphOS


#########################################################################
#
# Executables and objects for installation
#

# Main V executable
ifeq ($(BUILD), MORPHOS)
VEXE = V.morphos
endif
ifeq ($(BUILD), AMIGAOS)
VEXE = V.amigaos
endif

# Readme file
VREADME = V.readme

# Plugins
VPLUGINS = Plugins/Search.VPlug

# Image decoders, JScript and SSL
VLIBS = Plugins/vimgdecode_\#?.vlib Plugins/voyager_\#?.vlib

# MUI Custom Classes
MCC = ToolBar.mcc ListTree.mc? ToolButton.mcc BookMarks.mcc Textfield.mc?

# Icons
ICONS = V.info V020.info V.ReadMe.info MUI.info Voyager_Home.info Rexx.info Tools.info Install-Update.info Install.info


#########################################################################
#
# Targets
#
.PHONY: build all debug clean mrproper depend flat gst update alpha install checkdebug cleanobjs

ifeq ($(wildcard $(DEPFILE)),$(DEPFILE))
all: build
else
all: depend
	$(MAKE) -f $(MAKEFILENAME) build
endif

68k:
	$(MAKE) -f $(MAKEFILENAME) build BUILD=AMIGAOS

build:
	@echo ""
	@echo "Full build in progress, all your browsers are belong to us.."
	@echo ""
	@for i in $(CPU-TYPES); \
		do (echo ""; \
			echo -n "Making "; \
			echo -n $$i; \
			echo " version ..."; \
			echo ""; \
			mkdir -p $(BUILDDIR)/$$i; \
			cd $(BUILDDIR)/$$i; \
			copy //SCOPTIONS SCOPTIONS; \
			$(MAKE) -f ../../$(MAKEFILENAME) flat -I../.. CPU=$$i ) ; done

flat: prebuild_$(CVSUSERNAME) $(VEXE) postbuild_$(CVSUSERNAME)


# use the DB macro to output debuging stuff
debug:
	$(MAKE) VDEBUG=DEF=VDEBUG=2

gst: config.h imgstub.h voyager.gst

ifeq ($(BUILD), AMIGAOS)
$(VEXE): rev.h voyager.gst $(OBJS)
	slink \
from lib:c.o $(OBJS) \
to V.debug noicons \
addsym \
sc sd \
with lib:utillib.with \
lib $(LIBS) \
define _codeid=49 \
map v.map hsflxo swidth 32 noicons 
	slink v.debug to $(VEXE) stripdebug
endif

ifeq ($(BUILD), MORPHOS)
$(VEXE): $(OBJS)
	$(CROSSLINK) $(LDOPTS) $(VEXE).db $(OBJS) $(LIBS)
	strip --remove-section=.comment -o $(VEXE) $(VEXE).db
	protect $(VEXE) +e
endif

install:
	@echo "installing..."
	$(MAKE) -f $(MAKEFILENAME) install_$(CVSUSERNAME)

update:
	$(BUMPER) INCCOMP
	touch copyright.h

loader:
	m68k-amigaos-gcc -I/gg/os-include -nostdlib -DNAME=\"V\" -O -s -fomit-frame-pointer starter.c -o V

# Alpha version (ie. for ibeta members)
alpha: build
#	 copy objects/604e/v.debug v.debug-`cat .revinfo`
	lzx -r -x -a a ram:V`cat .revinfo`alpha objects/604e/$(VEXE) $(VREADME)
#	 mailout
	@echo "no mailout.. it's in ram:"

checkdebug:
	-grep _kprintf objects/68020/V.debug

cleanobjs:
	-$(DELETEALL) $(BUILDDIR)/$(ARCH)

clean:
	-$(DELETEALL) $(BUILDDIR)/$(ARCH)

mrproper: clean
	-$(DELETE) js_parser.c js_scanner.h css_scanner.h *.gst V V.debug
	-$(DELETE) .depend.morphos .depend.amigaos

ifeq ($(BUILD), MORPHOS)
depend:
	@echo "Generating dependencies..."
	@touch voyager_cat.h charmap.h
	sh mkdep.sh -f $(DEPFILE) $(DEPOPTS) $(DEPINCLUDES) $(patsubst %.o,%.c, $(COBJS)) $(patsubst %.o,%.c, $(PLATFORMOBJS)) $(patsubst %.o,%.c, $(AMIGAOBJS)) $(patsubst %.o,%.c, $(GENERICOBJS))
	@rm -f voyager_cat.h charmap.h
endif

ifeq ($(BUILD), AMIGAOS)
depend:
else
depend68k:
endif
	@echo "Generating dependencies... (might take a while)"
	sh mkdep.sh -f .depend.amigaos $(DEPOPTS) $(DEPINCLUDES) $(patsubst %.o,%.c, $(COBJS)) $(patsubst %.o,%.c, $(PLATFORMOBJS)) $(patsubst %.o,%.c, $(AMIGAOBJS)) $(patsubst %.o,%.s, $(ASMOBJS))

dump:
	ppc-morphos-objdump --section-headers --all-headers --reloc --disassemble-all --syms objects/604e/V.morphos.db >objects/604e/V.morphos.dump

imgdecoders:
	$(MAKE) -w -C imgdecode
	$(MAKE) -w -C imgdecode 68k

betarelease: imgdecoders build 68k loader
	@echo "Making public beta release.."
	-makedir release/voyager
	-makedir release/voyager/Plugins
	-makedir release/voyager/libs
	copy libs:vapor_toolkit.library release/voyager/libs
#	 copy sys:morphos/libs/vapor_toolkit.library release/voyager/libs/vapor_toolkit.library.elf
	copy imgdecode/objects/604e/vimgdec_604e.vlib release/voyager/Plugins
	copy imgdecode/objects/68020/vimgdec_68020.vlib release/voyager/Plugins
	copy imgdecode/objects/68030/vimgdec_68030fpu.vlib release/voyager/Plugins
	copy imgdecode/objects/68040/vimgdec_68040fpu.vlib release/voyager/Plugins
	copy imgdecode/objects/68060/vimgdec_68060.vlib release/voyager/Plugins
	copy V.ReadMe release/voyager
	copy /vat/VAT.ReadMe release/voyager/libs
	copy objects/604e/V.morphos release/voyager/V.MorphOS
	copy objects/68020/V.amigaos release/voyager/V.AmigaOS
	copy V release/voyager/V
	cd release; \
	lha -r a ram:v_`cat ../.revinfo`_beta.lha voyager; \
	cd ..;


ifeq ($(wildcard ../../$(DEPFILE)),../../$(DEPFILE))
include $(DEPFILE)
endif
include makefile.rules

ifeq ($(BUILD), AMIGAOS)
include makefile.68kasm
endif

#########################################################################
#
# Misc stuff
#

# Move everything to include3rd and use them from here
ppcinclude:
	cp imgdecode/clib/vimgdecode_protos.h $(INCLUDE3RD)clib
	cp imgdecode/fd/vimgdecode_lib.fd $(INCLUDE3RD)fd
	cp imgdecode/libraries/vimgdecode.h $(INCLUDE3RD)libraries
	cp aboutstuff/clib/v_about_protos.h $(INCLUDE3RD)clib
	cp aboutstuff/fd/v_about_lib.fd $(INCLUDE3RD)fd
	cp aboutstuff/libraries/v_about.h $(INCLUDE3RD)libraries
	cp pluginapi/clib/v_plugin_protos.h $(INCLUDE3RD)clib
	cp pluginapi/fd/v_plugin_lib.fd $(INCLUDE3RD)fd
	cp pluginapi/libraries/v_plugin.h $(INCLUDE3RD)libraries
	cp ../../mui/mui.h $(INCLUDE3RD)libraries
	cp ../../mui/muimaster_protos.h $(INCLUDE3RD)clib
	cp ../../mui/muimaster_lib.fd $(INCLUDE3RD)fd
	cp ../textinput/textinput_mcc.h $(INCLUDE3RD)mui
	cp ../tearoff/TearOffBay_mcc.h $(INCLUDE3RD)mui
	cp ../tearoff/tearoffpanel_mcc.h $(INCLUDE3RD)mui
	cp ../speedbar/SpeedBar_mcc.h $(INCLUDE3RD)mui
	cp ../speedbar/SpeedButton/SpeedButton_mcc.h $(INCLUDE3RD)mui
	cp ../popph/popplaceholder_mcc.h $(INCLUDE3RD)mui
	cp ../vat/clib/vat_protos.h $(INCLUDE3RD)clib
	cp ../vat/fd/vat_lib.fd $(INCLUDE3RD)fd
	cp ../vat/libraries/vat.h $(INCLUDE3RD)libraries
	cp ../cmanager/Library/CManager_lib.fd $(INCLUDE3RD)fd
	cp ../cmanager/Library/CManager_protos.h $(INCLUDE3RD)clib
	cp ../cmanager/CManager.h $(INCLUDE3RD)libraries
	cp ../cmanager/MCC/CManager_mcc.h $(INCLUDE3RD)mui
	$(MAKE) -w -C $(MOSINCLUDE) include3rdall


#########################################################################
#
# Pre build procedure - user settable
#

# Dictator Olli
prebuild_owagner:

# Steffistalker stuntzi
prebuild_stuntzi:

# Master of Makefiles Zapek
prebuild_zapek:

# Slacker KingGuppy
prebuild_kingguppy:

# Whiner Matt
prebuild_neko:

# Lego Entity
prebuild_entity:

# Default global rule
prebuild_:
	@echo "No specific prebuild target"


#########################################################################
#
# Post build procedure - user settable
#

# Dictator Olli
postbuild_owagner:
	copy V.morphos //

# Steffistalker stuntzi
postbuild_stuntzi:
	copy V.amigaos V.debug //

# Master of Makefiles Zapek
postbuild_zapek:

# Slacker KingGuppy
postbuild_kingguppy:
	@echo "Done..."
	copy V.amigaos V.debug Internet:WWW/Voyager

# Whiner Matt
postbuild_neko:
	copy V.amigaos V.debug Work:Net/V_Alpha/
	@echo "F'nished"

# Lego Entity
postbuild_entity:
	copy V.amigaos V.debug DH1:Comms/Voyager
	@echo "Okidoki"

# Default global rule
postbuild_:
	@echo "No specific postbuild target"


#########################################################################
#
# Installation - user settable
#

# Zapek
install_zapek:
#	 copy V to amitcp:/Voyager
ifeq ($(BUILD), MORPHOS)
	copy $(BUILDDIR)/604e/$(VEXE) to amitcp:/Voyager
else
	copy $(BUILDDIR)/68020/$(VEXE) to amitcp:/Voyager
endif

# Default installation
install_:
	@echo "Installing nowhere. Add yourself in the 'Installation' part of the Makefile"


#########################################################################
#
# Specific targets
#

# Zapek
zapinstall: all install

zapninstall: all
#    -amitcp:bin/netmount >NIL: vicviper dh0 rh0
	copy objects/68020/V to vicviper:HardDisk/communications/Voyager

zapninstallmetalion: all
	-amitcp:bin/netmount >NIL: metalion dh0 rh1
	copy objects/68020/V to rh1:communications/Voyager

# wee.. since I'm the only one doing that..
morphosbeta:
	@echo "Making MorphOS beta release.."
	-delete release/voyager all quiet
	-makedir release/voyager
	-makedir release/voyager/Plugins
	-makedir release/voyager/libs
	copy mossys:libs/vapor_toolkit.library release/voyager/libs
	copy imgdecode/objects/604e/vimgdec_604e.vlib release/voyager/Plugins
	copy objects/604e/V.MorphOS release/voyager
	copy V.ReadMe to release/voyager
	mkdir -p debug
	copy objects/604e/V.MorphOS.db debug/V.MorphOS_`cat .revinfo`.db
	copy imgdecode/objects/604e/vimgdec_604e.vlib.db debug/vimgdec_`cat .revinfo`.db
	-delete ram:v_`cat .revinfo`_beta.lha
	cd release; \
	lha -r a ram:v_`cat ../.revinfo`_beta.lha voyager; \
	cd ..;
	@echo "Done, it's in ram"
