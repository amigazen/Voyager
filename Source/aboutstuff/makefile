#
# $Id: makefile,v 1.2 2003/06/23 20:55:05 olli Exp $
#

APPNAME = voyager_about.vlib

BUILDDIR = objects

include makefile.objs

MAKEFILENAME = makefile

PATH=/bin:/c

MAKE = make

TMPDIR = /tmp/
SOURCE = ../../

PLATFORM=morphos
VPATH := $(SOURCE)
ARCH=60?e

EXE = $(APPNAME)

NATVCC = gcc
CROSSCC = ppc-morphos-gcc
HOSTAS = as
CROSSAS = ppc-morphos-as
HOSTLINK = ld
CROSSLINK = ppc-morphos-ld

OBJS = $(COBJS_NODEP) $(COBJS)

CPU-TYPES = 604e

INCLUDES = -I- -I$(SOURCE). \
		   -I$(SOURCE)../include
DEPINCLUDES = -I. \
			  -I../include

DEPOPTS = -DDEPEND -DDEBUG
DEPFILE =.depend.morphos

CCOPTS = -noixemul -c -g -O2 -nostdlib -DDEBUG -DBUILD_WBLIB -fomit-frame-pointer -mmultiple -mcpu=$(CPU-TYPES) \
	-Wformat -Wunused -Wuninitialized -Wstrict-prototypes \
	-Werror-implicit-function-declaration $(INCLUDES)
LDOPTS = -fl libnix -o
HOSTCCOPTS = -c -fomit-frame-pointer -DAMIGAOS


.PHONY: build all debug clean mrproper depend flat update install archive

ifeq ($(wildcard $(DEPFILE)),$(DEPFILE))
all: build
else
all: depend
	$(MAKE) -f $(MAKEFILENAME) build
endif

build:
	@echo ""
	@echo "Full build in progress.."
	@echo ""
	@for i in $(CPU-TYPES); \
		do (echo ""; \
			echo -n "Making "; \
			echo -n $$i; \
			echo " version ..."; \
			echo ""; \
			mkdir -p $(BUILDDIR)/$$i; \
			cd $(BUILDDIR)/$$i; \
			$(MAKE) -f ../../$(MAKEFILENAME) flat -I../.. CPU=$$i) ; done

flat: $(APPNAME)

# Module generation
#
$(APPNAME): $(OBJS)
	$(CROSSLINK) $(LDOPTS) $(EXE).db $(OBJS) -L/gg/morphos/lib $(LIBS) /gg/lib/gcc-lib/$(shell $(CC) -dumpmachine)/$(shell $(CC) --version)/libgcc.a
	ppc-morphos-strip --remove-section=.comment -o $(SOURCE)$(EXE) $(EXE).db

depend:
	rev
	@echo "Generating dependencies..."
	sh ../mkdep.sh -f $(DEPFILE) $(DEPOPTS) $(DEPINCLUDES) $(patsubst %.o,%.c, $(COBJS))

clean:
	-rm -rf $(BUILDDIR)

mrproper: clean
	-rm -f rev.h .depend.morphos

dump:
	ppc-morphos-objdump --section-headers --all-headers --reloc --disassemble-all --syms objects/604e/$(APPNAME).db >objects/604e/$(APPNAME).dump

install: all
	copy $(APPNAME) /plugins

update:
	../rev INCREV
	touch copyright.h

ifeq ($(wildcard ../../$(DEPFILE)),../../$(DEPFILE))
include $(DEPFILE)
endif
include makefile.rules

