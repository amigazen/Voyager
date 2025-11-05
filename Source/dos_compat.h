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


#ifndef DOS_COMPAT_H
#define DOS_COMPAT_H

/*
 * AmigaOS DOS calls -> CaOS DOS calls
 *
 * $Id: dos_compat.h,v 1.3 2001/07/01 22:02:41 owagner Exp $
 */

#include <modules/dos/dos.h>
#include <dos_lib_calls.h>

extern DOSBASE;

#define DOSTRUE TRUE

/*
 * Functions
 */
#define Open OpenFile
#define Close CloseFile
#define Read ReadFile
#define Write WriteFile
#define Seek SeekFile
#define Lock LockHandle
#define UnLock UnLockHandle

#define Examine(a,b) ExamineObject(a,b,sizeof(struct FileInfoBlock))
#define ExamineFH(a,b) ExamineObject(a,b,sizeof(struct FileInfoBlock))

#define FileNo DOSHandle_p
struct FileInfoBlock {
	ExamineData_s  exStruct;
	UBYTE exData[256];
};
#define fib_FileName exStruct.ed_Name
#define fib_Protection exStruct.ed_Protect
#define fib_EntryType exStruct.ed_Type
#define fib_Size exStruct.ed_Size
#define fib_NumBlocks SEE_DOS_COMPAT_H_NUMBLOCKS_NOT_DONE
#define fib_Date exStruct.ed_Date
#define fib_Comment exStruct.ed_Comment
#define fib_OwnerUID exStruct.ed_Owner
#define fib_OwnerGID exStruct.ed_Group

#define Delay( n ) Suspend( (SYSTICKS_PER_SECOND/50)*n, NOFLAGS )

/* Long word alignement (mainly used to get
 * FIB or DISK_INFO as auto variables)
 * This may be broken for MBX?
 */
#define D_S(type,name) char a_##name[sizeof(type)+3]; \
										   type *name = (type *)((LONG)(a_##name+3) & ~3);
#define SHARED_LOCK OM_SHLOCK

#define ExAllControl ExamineControl
#define ExAllData ExamineData
#define eac_MatchString ec_MatchString
#define eac_LastKey ec_LastKey
#define eac_Entries ec_Entries
#define eac_MatchFunc ec_MatchFunc

#define IoErr GetDosError

#endif /* DOS_COMPAT_H */
