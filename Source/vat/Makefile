# Makefile for vapor_toolkit.library
#
# © 2001 VaporWare CVS team <ibcvs@vapor.com>
# All rights reserved
#
# $Id: Makefile,v 1.10 2002/10/28 14:35:52 zapek Exp $
#

SOURCE = ../../
BUILDDIR = objects
CPU-TYPES = 604e
VPATH := $(SOURCE)


CC = ppc-morphos-gcc
LD = ppc-morphos-ld

INCLUDES = -I- -I$(SOURCE). -I$(SOURCE)../include \
		   -I/gg/morphos/emulinclude/includegcc \
		   -I/gg/morphos/emulinclude/includestd

CFLAGS = -DDEBUG=0 -g -c -O2 -nostdlib -fomit-frame-pointer -mmultiple -mcpu=$(CPU-TYPES) $(INCLUDES)
LDFLAGS = -v -L/gg/morphos/lib $(OBJS) -lamiga -lstringio -lmath -lmoto -lsyscall -lgmp -lstring -lc -lamigastubs


# important: noexec.o has to be linked *first*
OBJS = noexec.o vat.o asyncio.o sup_generic.o md5.o random.o string.o time.o mui.o mime.o tcpip.o malloc.o \
	   libfunctions.o libfunctable.o vat_ctype.o lib.o


#########################################################################
#
# Targets
#
.PHONY: build all debug clean mrproper

build:
	@echo ""
	@echo "Full build in progress... Warning: no dependencies !"
	@echo ""
	@for i in $(CPU-TYPES); \
		do (echo ""; \
			echo -n "Making "; \
			echo -n "$$i"; \
			echo " version ..."; \
			echo ""; \
			mkdir -p $(BUILDDIR)/$$i; \
			cd $(BUILDDIR)/$$i; \
			$(MAKE) -f ../../Makefile flat -I../.. CPU=$$i ) ; done

flat: vapor_toolkit.library.elf

clean:
	-rm -r $(BUILDDIR)

rev.h: .revinfo
	rev DIRECTORY=//

mrproper: clean

install: build
	-copy $(BUILDDIR)/604e/vapor_toolkit.library.elf to libs:
	-flushlib vapor_toolkit.library

libfunctions.c: fd/vat_lib.fd clib/vat_protos.h
	cvinclude.pl --libprefix=LIB_ --fd $(SOURCE)fd/vat_lib.fd --clib $(SOURCE)clib/vat_protos.h --gatestubs $(SOURCE)$@_tmp
	cat $(SOURCE)$@_tmp | sed -e "s,../../,," > $(SOURCE)$@
	rm -f $(SOURCE)$@_tmp
#	 fd2inline --morphos --gatestubs --prefix LIB_ $(SOURCE)fd/vat_lib.fd $(SOURCE)clib/vat_protos.h -o $(SOURCE)$@

vapor_toolkit.library.elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@.db
	strip --remove-section=.comment $@.db -o $@

dump:
	ppc-morphos-objdump --section-headers --all-headers --reloc --disassemble-all --syms objects/604e/vapor_toolkit.library.elf.db >objects/604e/vapor_toolkit.elf.dump


#
# Dependencies (sort of)
#

.c.o: $*.o
	$(CC) $(CFLAGS) $(DEBUG) $(SOURCE)$*.c

