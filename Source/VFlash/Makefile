# Makefile for VFlash
#
# © 1999-2001 David Gerber <zapek@vapor.com>
# All rights reserved
#
# $Id: Makefile,v 1.8 2004/11/30 03:45:02 henes Exp $
#

#########################################################################
#
# Environment
#
#PATH=/bin:/sc/c:/c:/amitcp/bin

CC = gcc
LD = ld
OPTIMIZE = -O2
CFLAGS = $(OPTIMIZE) -fomit-frame-pointer
LDFLAGS =

BUILDDIR = objects
SOURCE = ../../
VPATH = $(SOURCE)

#CPU-TYPES = 020 030fpu 040fpu 060 powerup morphos #debug
CPU-TYPES = morphos

export CC PATH CFLAGS LDFLAGS SOURCE

#########################################################################
#
# Targets
#
.PHONY: build plugin archive ppccheck debug update clean dump

all: rev build plugin

release: build plugin archive

build:
	@echo ""
	@echo "Making full build, go eat a pizza or something"
	@echo ""
	@for i in $(CPU-TYPES); \
		do (echo ""; \
			echo -n "Making "; \
			echo -n $$i; \
			echo " version ..."; \
			echo ""; \
			mkdir -p $(BUILDDIR)/$$i; \
			cd $(BUILDDIR)/$$i; \
			$(MAKE) -f ../../Makefile.$$i ) ; done

archive-lzx:
	-delete vflash_`cat .revinfo`.lzx
	lzx -9 a vflash_`cat .revinfo` build/020/VFlash_68020.module build/030fpu/VFlash_68030fpu.module build/040fpu/VFlash_68040fpu.module build/060/VFlash_68060.module build/ppc/VFlash_ppc.module VFlash.readme VFlash.guide VFlash.guide VFlash.guide.info plugin/VFlash.VPlug Install_Plugins Install_Plugins.info
	lzx a vflash_`cat .revinfo` /flushlib/flushlib

archive-lha:
	-delete vflash_`cat .revinfo`.lha
	lha a vflash_`cat .revinfo`.lha build/020/VFlash_68020.module build/030fpu/VFlash_68030fpu.module build/040fpu/VFlash_68040fpu.module build/060/VFlash_68060.module build/ppc/VFlash_ppc.module VFlash.readme VFlash.guide VFlash.guide VFlash.guide.info plugin/VFlash.VPlug Install_Plugins Install_Plugins.info
	lha a vflash_`cat .revinfo`.lha /flushlib/flushlib

archive-tgz: all
	-delete vflash all
	makedir vflash
	makedir vflash/plugins
	copy VFlash.readme VFlash.guide vflash
	copy objects/morphos/VFlash_m604e.module vflash/plugins
	copy plugin/VFlash.VPlug.elf vflash/plugins/VFlash.VPlug
	tar -czvf ram:vflash_`cat .revinfo`.tar.gz vflash
	-delete vflash all

archive: archive-tgz

ppccheck:
	ppc-morphos-objdump --section-headers --all-headers --reloc --disassemble-all --syms build/ppc/VFlash_ppc.module | grep UND | grep -v PPC

plugin:
	make -C plugin
#	 @rev libid=flash_plugin GLOBAL QUIET
#	 cd plugin; sc:c/smake

rev:
	@rev

update:
	rev INCREV

clean:
	@-for i in $(CPU-TYPES); \
		do (mkdir -p $(BUILDDIR)/$$i; \
			cd $(BUILDDIR)/$$i; \
			$(MAKE) -f ../../Makefile.$$i clean) ; done
	cd plugin; sc:c/smake clean

install: all
ifeq ($(USER),zapek)
	copy objects/morphos/VFlash_m604e.module amitcp:/voyager/plugins
	copy plugin/VFlash.VPlug.elf amitcp:/voyager/plugins/VFlash.VPlug
	-flushlib VFlash.VPlug
else
	@echo "oh well.."
endif

install-iso:
	@echo "not for the ISO.. skipping"

#
# Private installation stuff for Zapek, add your own one below
#
zapinstall: all
	@-for i in $(CPU-TYPES); \
		do (copy $(BUILDDIR)/$$i/*.module AmiTCP:/Voyager/Plugins; echo "Installed $$i") ; done
	@copy plugin/VFlash.VPlug AmiTCP:/Voyager/Plugins
	@echo "Installed VFlash.VPlug"

zapninstall: all
	@-netmount >NIL: metalion dh0 rh0
	@-for i in $(CPU-TYPES); \
		do (copy $(BUILDDIR)/$$i/*.module rh0:communications/Voyager/Plugins) ; done
	@copy plugin/VFlash.VPlug rh0:communications/Voyager/Plugins

dump:
	ppc-morphos-objdump --section-headers --all-headers --reloc --disassemble-all --syms objects/morphos/VFlash_m604e.module.debug >objects/morphos/VFlash_m604e.module.dump

