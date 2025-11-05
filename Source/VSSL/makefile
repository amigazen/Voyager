OBJ = vssl.o bss_sock_amiga.o ftzs.o bss_file.o

voyager_ssl.vlib: $(OBJ) os_ssl.lib os_crypt.lib
	slink <with <
from lib:libent.o,lib:libinitr.o,$(OBJ)
to voyager_ssl.vlib
libversion 8
librevision 6
libid "voyager_ssl.vlib 8.6 (19.03.2000)"
libfd vssl_lib.fd
sc sd addsym
with lib:utillib.with
lib os_ssl.lib os_crypt.lib
lib lib:sc.lib lib:debug.lib lib:small.lib
map ssl.map hsflxo swidth 32
DEFINE @__chkabort=@__dummy
DEFINE ___chkabort=@__dummy
<
	slink voyager_ssl.vlib to /plugins/voyager_ssl.vlib nd
    flushlib voyager_ssl.vlib

ftzs.o: ftzs.s
	asm ftzs.s

