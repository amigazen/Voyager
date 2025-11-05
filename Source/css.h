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


/*
**
**	$Id: css.h,v 1.6 2001/07/01 22:02:37 owagner Exp $
**
*/

// A single style specfication
struct style {
	char *name;				// CSS class name
	char *font_spec;
	int font_decoration;
	int font_height;
	int margintop, marginleft, marginright, marginbottom;
};

// Style sheet, as active at a current layout state
struct stylesheet {
	struct style def;
	struct style h[ 6 ];
	struct style td;
	struct style a;
	struct style a_hover;
	struct style a_visited;
};
