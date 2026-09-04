OBJ = mimeprefs.o loadvprefs.o mimeprefs_cat.o

mimeprefs: $(OBJ)
	slink <with <
from lib:c.o $(OBJ)
to mimeprefs.debug
sc sd addsym
with lib:utillib.with
lib lib:vatlib.lib lib:sc.lib lib:small.lib lib:debug.lib
map mp.map hsflxo swidth 32
<
	slink mimeprefs.debug to MimePrefs nd

mimeprefs.gst: gst.h 
	gst unload mimeprefs.gst
	sc makegst=mimeprefs.gst gst.c

mimeprefs_cat.o: mimeprefs.cd
	catmaker mimeprefs.cd mimeprefs_cat.h
	asm -omimeprefs_cat.o t:cattmp.a
	delete t:cattmp.a

loadvprefs.o: loadvprefs.c
	sc loadvprefs.c

mimeprefs.o: mimeprefs.c mimeprefs.h mimeprefs.gst mimeprefs_cat.o
	sc mimeprefs.c
