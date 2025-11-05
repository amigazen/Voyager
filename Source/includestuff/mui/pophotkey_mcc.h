#pragma pack(2)
#pragma pack(2)
/*

		MCC_Pophotkey (c) by kMel, Klaus Melchior

		Registered class of the Magic User Interface.

		Pophotkey_mcc.h

*/


/*** Include stuff ***/

#ifndef POPHOTKEY_MCC_H
#define POPHOTKEY_MCC_H

#ifndef LIBRARIES_MUI_H
#pragma pack()
#pragma pack()
#include "libraries/mui.h"
#pragma pack(2)
#pragma pack(2)
#endif


/*** MUI Defines ***/

#ifdef _DCC
 extern const char MUIC_Pophotkey[];
#else
# define MUIC_Pophotkey "Pophotkey.mcc"
#endif

#define PophotkeyObject MUI_NewObject(MUIC_Pophotkey



/*** Methods ***/

#define MUIM_Pophotkey_Close          0x8002009f
#define MUIM_Pophotkey_Open           0x800200a0

/*** Method structs ***/

struct MUIP_Pophotkey_Close {
	ULONG MethodID;
	ULONG Type;
};

struct MUIP_Pophotkey_Open {
	ULONG MethodID;
};


/*** Special method values ***/

#define MUIV_Pophotkey_Close_Type_Cancel            0
#define MUIV_Pophotkey_Close_Type_Ok                1


/*** Special method flags ***/



/*** Attributes ***/

#define MUIA_Pophotkey_Extended       0x800200a4

/*** Special attribute values ***/



/*** Structures, Flags & Values ***/





#endif /* POPHOTKEY_MCC_H */

/* PrMake.rexx 0.10 (16.2.1996) Copyright 1995 kmel, Klaus Melchior */


#pragma pack()
#pragma pack()
