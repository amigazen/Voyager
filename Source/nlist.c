/**************************************************************************

  =======================
  The Voyager Web Browser
  =======================

  Copyright (C) 1995-2003 by
   Oliver Wagner <owagner@vapor.com>
   All Rights Reserved

  Parts Copyright (C) by
   David Gerber <zapek@vapor.com>
   Jon Bright <jon@siliconcircus.com>
   Matt Sealey <neko@vapor.com>

**************************************************************************/


/*
 * NList support
 * -------------
 * © 2001 by VaporWare
 * All Rights Reserved
 *
 * $Id: nlist.c,v 1.3 2003/07/06 16:51:34 olli Exp $
 */

#include "voyager.h"

/* private */
#include "mui_func.h"
#include "nlist.h"

char *listclass = MUIC_List;
char *listviewclass = MUIC_Listview;
int  nlist;

#if USE_NLIST
/* checks if NList is available and sets some global vars */
void check_for_nlist( void )
{
	APTR to;

	if( to = MUI_NewObject( MUIC_NList, NULL ) )
	{
		MUI_DisposeObject( to );
		listclass = MUIC_NList;
		listviewclass = MUIC_NListview;
		nlist = TRUE;
	}
}
#endif
