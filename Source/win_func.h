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
 * $Id: win_func.h,v 1.10 2001/07/01 22:03:33 owagner Exp $
 */

ULONG get_window_number( void );
void add_window_menu( APTR obj, STRPTR name, ULONG num );
void add_window_menu_extended( APTR obj, APTR winobj, STRPTR name, ULONG num , int settitle);
void remove_window_menu( APTR obj );
void set_window_title( APTR obj, STRPTR name );
ULONG get_active_window( void );
APTR win_find_by_name( STRPTR name, int toponly );
void checkwinremove( void );
void STDARGS doallwins( ULONG methodid, ... );
#ifdef __MORPHOS__
#define doallwins(...) _doallwins(0, __VA_ARGS__ )
void _doallwins(int stub, ...) __attribute__((varargs68k));
#endif /* __MORPHOS__ */
void doallwins_a( Msg msg );
void cleanup_windows( void );
void set_window_close( APTR obj );
int	donumwins_a( ULONG num, Msg msg );
APTR get_window_by_ix( int ix );
STRPTR get_window_menu_title( int ix );
void set_smooth_scroll( APTR obj, int mode );
