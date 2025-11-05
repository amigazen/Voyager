#
# libmath64 - a SAS/C math linklib with 64-bit support
#
# © 2001 by David Gerber <zapek@meanmachine.ch>
# All Rights Reserved
#
# $Id: makefile,v 1.5 2001/09/27 14:00:44 zapek Exp $

OBJS = qtod.o q_add.o q_div.o q_div020.o q_div64.o q_divu64.o q_divu.o q_divu020.o q_mod.o \
	q_mod020.o q_modu.o q_mul020.o q_sub.o uqtoud.o q_mulu.o
ASM = sc
CC = sc
CFLAGS = OBJECTNAME=""

PATH=/bin:/sc/c:/c
MATH-TYPES = NOMATH IEEE
BUILDDIR = objects
MAKEFILENAME = makefile
VPATH = ../../
SOURCE = //

all: build

build:
	@echo ""
	@echo "Full build in progress.."
	@echo ""
	@for i in $(MATH-TYPES); \
		do (echo ""; \
			echo -n "Making "; \
			echo -n $$i; \
			echo " version ..."; \
			echo ""; \
			mkdir -p $(BUILDDIR)/$$i; \
			cd $(BUILDDIR)/$$i; \
			copy //SCOPTIONS SCOPTIONS; \
			$(MAKE) -f ../../$(MAKEFILENAME) flat MATH=$$i ) ; done

.s.o: $*.s
	$(ASM) $(SOURCE)$*.s
	@mv ../../$*.o .

.c.o: $*.c
	$(CC) $(CFLAGS) $(SOURCE)$*.c

flat: prebuild lib postbuild

prebuild:
ifneq ($(MATH),NOMATH)
	@echo -n "MATH=" >>SCOPTIONS
	@echo "$(MATH)" >>SCOPTIONS
endif

postbuild:
ifeq ($(MATH),NOMATH)
	@mv libmath64.lib ../../libmath64.lib
endif
ifeq ($(MATH),IEEE)
	@mv libmath64.lib ../../libmath64ieee.lib
endif

lib: $(OBJS)
	-delete libmath64.lib QUIET
	oml libmath64.lib a $(OBJS)

test: test.c libmath64ieee.lib
	$(CC) MATH=IEEE test.c link LIB libmath64ieee.lib

clean:
	-delete objects test.o test.lnk test libmath64\#?.lib ALL QUIET

