/*
 * VFlash support functions
 * ------------------------
 * - small functions
 *
 * $Id: support.c,v 1.3 2004/04/15 19:33:10 zapek Exp $
 *
 */

#include "globals.h"
#include "support.h"

#if defined(AMIGAOS) || defined(__MORPHOS__)
APTR oldwinptr;
#endif

/*
 * Support functions
 */
ULONG getv(APTR obj, ULONG attr)
{
	ULONG v;
	struct opGet g;

	g.MethodID = OM_GET;
	g.opg_AttrID = attr;
	g.opg_Storage = &v;
	ft->vplug_domethoda(obj, &g);

	return (v);
}

void Fail(STRPTR txt)
{
	MUI_Request(NULL, NULL, 0, "VFlash Error", "Argh", txt, 0);
}

