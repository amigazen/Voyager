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
 * Specialized MUI macros
 * ----------------------
 * - Mainly used for MorphOS
 *
 * $Id: mui_macros.h,v 1.3 2001/07/01 22:03:16 owagner Exp $
 */

#ifdef __MORPHOS__


/*
 * Boopsi Macros (not used anymore. we use stubs now)
 */

#if 0
#define CallHook(MyHook, MyObject, tags...) \
	({ULONG _tags[] = { tags }; CallHookA((MyHook), (MyObject), (APTR)_tags);})

#define DoMethod(MyObject, tags...) \
	({ULONG _tags[] = { tags }; DoMethodA((MyObject), (APTR)_tags);})

#define DoSuperMethod(MyClass, MyObject, tags...) \
	({ULONG _tags[] = { tags }; DoSuperMethodA((MyClass), (MyObject), (APTR)_tags);})

#define CoerceMethod(MyClass, MyObject, tags...) \
	({ULONG _tags[] = { tags }; CoerceMethodA((MyClass), (MyObject), (APTR)_tags);})

#define SetSuperAttrs(Class, MyObject, tags...)				\
({									\
ULONG		_tags[] = { tags }; 					\
struct opSet	MyopSet;						\
  MyopSet.MethodID	=	OM_SET;					\
  MyopSet.ops_AttrList	=(struct TagItem*) _tags;			\
  MyopSet.ops_GInfo	=	NULL;					\
  DoSuperMethodA((Class), (MyObject), (APTR) &MyopSet);			\
})

#define DoSuperNew(MyClass, MyObject, tags...) \
({									\
ULONG			_tags[] = { tags }; 				\
struct opSet		MyopSet;					\
  MyopSet.MethodID	=	OM_NEW;					\
  MyopSet.ops_AttrList	=(struct TagItem*) _tags;			\
  MyopSet.ops_GInfo	=	NULL;					\
  DoSuperMethodA((MyClass), (MyObject), (APTR) &MyopSet);		\
})
#endif

#endif /* __MORPHOS__ */
