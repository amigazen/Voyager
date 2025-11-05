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
 * template functions
 * ------------------
 * - functions to expand templates
 *
 * © 2001 by VaporWare
 * All Rights Reserved
 *
 * $Id: template.c,v 1.4 2003/07/06 16:51:34 olli Exp $
 */

#include "voyager.h"

/*
 * Transforms the 'from' string ito a 'to' string of 'maxlen'
 * expanding the template given as pair argument in the vararg. First argument
 * is the one letter to replace, second argument is the replacement, eg:
 * 'f', fullpath.
 */
int STDARGS expandtemplate( char *from, char *to, int maxlen, ... )
{
	int idp;
	char *datap;
	int ch;
	char *oldto = to;
	va_list va;


	while( *from )
	{
		if( *from != '%' )
		{
			*to++ = *from++;
			continue;
		}
		from++;
		ch = *from++;

		va_start( va, maxlen );

		idp = va_arg( va, int );
		datap = va_arg( va, char * );

		while( idp )
		{
			/* replacement char found */
			if( idp == ch )
			{
				if( datap )
					strcpy( to, datap );
				to = strchr( to, 0 );
				break;
			}
			idp = va_arg( va, int );
			datap = va_arg( va, char * );
		}
		va_end( va );
		/* not found, Zeichen 1:1 übernehmen, zum donnerwetter ! */
		if( !idp )
			*to++ = ch;
	}
	*to = 0;

	va_end( va );

	return( (int)strlen( oldto ) );
}

