/**************************************************************************

  =======================
  The Voyager Web Browser
  =======================

  Copyright (C) 1995-2001 by
   Oliver Wagner <owagner@vapor.com>
   All Rights Reserved

  Parts Copyright (C) by
   David Gerber <zapek@vapor.com>
   Jon Bright <jon@siliconcircus.com>
   Matt Sealey <neko@vapor.com>

**************************************************************************/


#ifndef VOYAGER_REDRAW_H
#define VOYAGER_REDRAW_H
/*
 * $Id: redraw.h,v 1.5 2001/07/01 22:03:22 owagner Exp $
 */

static void __inline mui_vredraw( Object *obj, ULONG flags )
{
	if(_isvisible(obj))
	{
		_flags(obj) &= ~MADF_DRAWMASK;
		_flags(obj) |= flags;

		while (_isinvirtual(obj) && _parent(obj) && ( _flags( obj ) & MADF_PARTIAL ) )
		{
			obj = _parent(obj);
			_flags(obj) &= ~MADF_DRAWMASK;
			_flags(obj) |= MADF_DRAWCHILD;
		}

		DoMethod(obj,MUIM_Draw,0);
	}
}

#endif /* VOYAGER_REDRAW_H */
