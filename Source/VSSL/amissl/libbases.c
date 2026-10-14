/*
 * Copyright (C) 2026, amigazen project
 */

/*
 * SAS/C #pragma libcall bases for amisslmaster.library, amissl.library
 * and bsdsocket.library.  AmiSSL v5 splits OpenSSL across two bases;
 * OpenAmiSSLTagList writes both pointers.
 */

#include <exec/types.h>
#include <exec/libraries.h>

struct Library *AmiSSLMasterBase;
struct Library *AmiSSLBase;
struct Library *AmiSSLExtBase;
struct Library *SocketBase;
int errno;
int h_errno;

/*
 * sc.lib _exit.c references __XCEXIT.  SAS/C adds one underscore to the
 * C name, so the function is spelled _XCEXIT.  A shared library must not
 * kill the process.
 */
void __saveds _XCEXIT(void)
{
}

void __regargs __chkabort(void)
{
}

void __regargs _CXBRK(void)
{
}
