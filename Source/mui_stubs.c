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
 * Stubs for MorphOS. Well, since it took me
 * a long time to find out, let's take an example
 * with DoSuperNew():
 *
 * $Id: mui_stubs.c,v 1.3 2003/07/06 16:51:34 olli Exp $
 */

#include "voyager.h"

#include "mui_func.h"

#if 0
/*
 * Either you use a macro which forces the parameters to
 * be in a LINEAR array (like a 68k stack):
 */

#define DoSuperNew(MyClass, MyObject, tags...) \
({									\
ULONG			_tags[] = { tags }; 				\
struct opSet		MyopSet;					\
  MyopSet.MethodID	=	OM_NEW;					\
  MyopSet.ops_AttrList	=(struct TagItem*) _tags;			\
  MyopSet.ops_GInfo	=	NULL;					\
  DoSuperMethodA((MyClass), (MyObject), (APTR) &MyopSet);		\
})

/*
 * This has several disadvantages. You can't put #ifdef within a
 * DoSuperNew() call and you have warnings from gcc if the args
 * are not ULONG.
 */

/*
 * The best way is to avoid to use a macro and use a vararg
 * stub with vararg68k gcc-extention instead. Don't forget
 * vararg68k in the prototype of the function ! (mui_func.h)
 */
#endif

APTR STDARGS DoSuperNew( struct IClass *cl, APTR obj, ... )
{
	struct opSet os;
	APTR rc;
	va_list va;
	va_start( va, obj );
	os.MethodID = OM_NEW;
	os.ops_AttrList = ( struct TagItem * )va->overflow_arg_area;
	os.ops_GInfo = NULL;
	rc = ( APTR )DoSuperMethodA( cl, obj, (Msg)&os );
	va_end( va );
	return( rc );
}

ULONG CallHook( struct Hook *hookPtr, Object *obj, ... )
{
	va_list va;
	ULONG rc;
	va_start( va, obj );
	rc = CallHookA( hookPtr, obj, ( APTR )va->overflow_arg_area );
	va_end( va );
	return( rc );
}

ULONG DoMethod( Object *obj, ... )
{
	va_list va;
	ULONG rc;
	va_start( va, obj );
	rc = DoMethodA( obj, ( Msg )va->overflow_arg_area );
	va_end( va );
	return( rc );
}

ULONG DoSuperMethod( struct IClass *cl, Object *obj, ... )
{
	va_list va;
	ULONG rc;
	va_start( va, obj );
	rc = DoSuperMethodA( cl, obj, ( Msg )va->overflow_arg_area );
	va_end( va );
	return( rc );
}

ULONG CoerceMethod( struct IClass *cl, Object *obj,	... )
{
	va_list va;
	ULONG rc;
	va_start( va, obj );
	rc = CoerceMethodA( cl, obj, ( Msg )va->overflow_arg_area );
	va_end( va );
	return( rc );
}

ULONG SetSuperAttrs( struct IClass *cl, Object *obj, ... )
{
	struct opSet os;
	ULONG rc;
	va_list va;
	va_start( va, obj );
	os.MethodID = OM_SET;
	os.ops_AttrList = ( struct TagItem * )va->overflow_arg_area;
	os.ops_GInfo = NULL;
	rc = DoSuperMethodA( cl, obj, (Msg)&os );
	va_end( va );
	return( rc );
}


