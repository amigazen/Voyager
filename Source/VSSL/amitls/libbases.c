/*
 * SAS/C #pragma libcall for amitls.library and bsdsocket.library
 * resolve through these near-data bases.
 */

#include <exec/types.h>
#include <exec/libraries.h>

struct Library *TlsBase;
struct Library *SocketBase;
int errno;
int h_errno;

/* sc.lib _exit / abort hooks — shared libraries must not terminate the process. */
void __saveds __XCEXIT(void)
{
}

void __regargs __chkabort(void)
{
}

void __regargs _CXBRK(void)
{
}
