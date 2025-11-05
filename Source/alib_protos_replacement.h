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
 * $Id: alib_protos_replacement.h,v 1.2 2001/07/01 22:02:35 owagner Exp $
 */
#ifdef CLIB_ALIB_PROTOS_H
#error clib/alib_protos.h must NOT be included in any way. please fix
#endif /* CLIB_ALIB_PROTOS_H */

#ifndef  VOYAGER_ALIB_PROTOS_REPLACEMENT_H
#define  VOYAGER_ALIB_PROTOS_REPLACEMENT_H

struct Hook;
struct IORequest;
struct InputEvent;
struct timeval;
struct Task;
struct Message;
struct KeyMap;
struct IOStdReq;
struct MsgPort;
struct List;
struct Isrvstr;
struct IClass;


#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  DEVICES_TIMER_H
#include <devices/timer.h>
#endif
#ifndef  DEVICES_KEYMAP_H
#include <devices/keymap.h>
#endif
#ifndef  LIBRARIES_COMMODITIES_H
#include <libraries/commodities.h>
#endif
#ifndef  UTILITY_HOOKS_H
#include <utility/hooks.h>
#endif
#ifndef  INTUITION_CLASSES_H
#include <intuition/classes.h>
#endif
#ifndef  INTUITION_CLASSUSR_H
#include <intuition/classusr.h>
#endif
#ifndef  GRAPHICS_GRAPHINT_H
#include <graphics/graphint.h>
#endif



void BeginIO( struct IORequest *ioReq );
struct IORequest *CreateExtIO( struct MsgPort *port, long ioSize );
struct MsgPort *CreatePort( STRPTR name, long pri );
struct IOStdReq *CreateStdIO( struct MsgPort *port );
struct Task *CreateTask( STRPTR name, long pri, APTR initPC,
	unsigned long stackSize );
void DeleteExtIO( struct IORequest *ioReq );
void DeletePort( struct MsgPort *ioReq );
void DeleteStdIO( struct IOStdReq *ioReq );
void DeleteTask( struct Task *task );
void NewList( struct List *list );
APTR LibAllocPooled( APTR poolHeader, unsigned long memSize );
APTR LibCreatePool( unsigned long memFlags, unsigned long puddleSize,
	unsigned long threshSize );
void LibDeletePool( APTR poolHeader );
void LibFreePooled( APTR poolHeader, APTR memory, unsigned long memSize );



ULONG FastRand( unsigned long seed );
UWORD RangeRand( unsigned long maxValue );



void AddTOF( struct Isrvstr *i, long (*p)(void), long a );
void RemTOF( struct Isrvstr *i );
void waitbeam( long b );



FLOAT afp( BYTE *string );
void arnd( long place, long exp, BYTE *string );
FLOAT dbf( unsigned long exp, unsigned long mant );
LONG fpa( FLOAT fnum, BYTE *string );
void fpbcd( FLOAT fnum, BYTE *string );



LONG TimeDelay( long unit, unsigned long secs, unsigned long microsecs );
LONG DoTimer( struct timeval *, long unit, long command );


void ArgArrayDone( void );
UBYTE **ArgArrayInit( long argc, UBYTE **argv );
LONG ArgInt( UBYTE **tt, STRPTR entry, long defaultval );
STRPTR ArgString( UBYTE **tt, STRPTR entry, STRPTR defaulstring );
CxObj *HotKey( STRPTR description, struct MsgPort *port, long id );
struct InputEvent *InvertString( STRPTR str, struct KeyMap *km );
void FreeIEvents( struct InputEvent *events );



BOOL CheckRexxMsg( struct Message *rexxmsg );
LONG GetRexxVar( struct Message *rexxmsg, UBYTE *name, UBYTE **result );
LONG SetRexxVar( struct Message *rexxmsg, UBYTE *name, UBYTE *value,
	long length );



ULONG CallHookA( struct Hook *hookPtr, Object *obj, APTR message );
ULONG CallHook( struct Hook *hookPtr, Object *obj, ... ) __attribute__((varargs68k));
ULONG DoMethodA( Object *obj, Msg message );
ULONG DoMethod( Object *obj, ... ) __attribute__((varargs68k));
ULONG DoSuperMethodA( struct IClass *cl, Object *obj, Msg message );
ULONG DoSuperMethod( struct IClass *cl, Object *obj, ... ) __attribute__((varargs68k));
ULONG CoerceMethodA( struct IClass *cl, Object *obj, Msg message );
ULONG CoerceMethod( struct IClass *cl, Object *obj,	... ) __attribute__((varargs68k));
ULONG SetSuperAttrs( struct IClass *cl, Object *obj, ... ) __attribute__((varargs68k));


STRPTR ACrypt( STRPTR buffer, STRPTR password, STRPTR username );
#endif /* VOYAGER_ALIB_PROTOS_REPLACEMENT_H */
