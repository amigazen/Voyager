
CC = ppc-morphos-gcc
AR = ppc-morphos-ar
CFLAGS = -O2 -Wall -noixemul -I../

OBJS = vatlib.o open_iff.o open_icon.o open_rexx.o open_datatypes.o open_diskfont.o

libvat.a: $(OBJS)
	rm -f $@
	$(AR) r $@ $(OBJS)

clean:
	rm -f libvat.a *.o
