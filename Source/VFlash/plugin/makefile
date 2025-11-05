
APPNAME = VFlash.VPlug.elf

BUILDDIR = objects

include makefile.objs

MAKEFILENAME = makefile

PATH=/bin:/c

MAKE = make

DELETE = rm -f
DELETEALL = rm -rf
TMPDIR = /tmp/
SOURCE = ../../

VPATH := $(SOURCE)

EXE = $(APPNAME)

NATVCC = gcc
CROSSCC = ppc-morphos-gcc
HOSTAS = as
CROSSAS = ppc-morphos-as
HOSTLINK = ld
CROSSLINK = ppc-morphos-ld

OBJS = $(COBJS) $(COBJS_NODEP)

CPU-TYPES = 604e

INCLUDES = -I$(SOURCE)include -I$(SOURCE).. -I$(SOURCE)../Lib
DEPINCLUDES = -Iinclude -I.. -I../Lib

DEPOPTS = -DDEPEND
DEPFILE = .depend

CCOPTS = -noixemul -DDEBUG=0 -c -O2 -fomit-frame-pointer -mmultiple -mcpu=$(CPU-TYPES) \
	-Wformat -Wunused -Wuninitialized -Wstrict-prototypes \
	-Werror-implicit-function-declaration $(INCLUDES)
LDOPTS = -fl libnix -o
HOSTCCOPTS = -c -fomit-frame-pointer

.PHONY: build all clean mrproper depend flat install archive

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
			$(MAKE) -f ../../$(MAKEFILENAME) flat -I../.. CPU=$$i BUILD=$(BUILD)) ; done

flat: $(APPNAME)

$(APPNAME): $(OBJS)
	$(CROSSLINK) $(LDOPTS) $(EXE) $(OBJS) $(LIBS)
	cp $(EXE) $(SOURCE)
	strip --remove-section=.comment $(SOURCE)$(EXE)

depend:
	@echo "Generating dependencies..."
	sh mkdep.sh -f $(DEPFILE) $(DEPOPTS) $(DEPINCLUDES) $(patsubst %.o,%.c, $(COBJS))

clean:
	rm -rf $(BUILDDIR)

mrproper: clean
	rm rev.h

dump:
	ppc-morphos-objdump --section-headers --all-headers --reloc --disassemble-all --syms objects/604e/$(APPNAME) >objects/604e/$(APPNAME).dump

archive: build
	@echo "no rule yet"

install: all
	cp VFlash.VPlug.elf sys:communications/internet/voyager/plugins/VFlash.VPlug
	flushlib VFlash.VPlug

ifeq ($(wildcard ../../$(DEPFILE)),../../$(DEPFILE))
include $(DEPFILE)
endif
include makefile.rules

