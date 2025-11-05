#pragma pack(2)
#pragma pack(2)
/*

		MCC_Popposition (c) by kMel, Klaus Melchior

		Registered class of the Magic User Interface.

		Popposition_mcc.h

*/


/*** Include stuff ***/

#ifndef POPPOSITION_MCC_H
#define POPPOSITION_MCC_H

#ifndef LIBRARIES_MUI_H
#pragma pack()
#pragma pack()
#include "libraries/mui.h"
#pragma pack(2)
#pragma pack(2)
#endif


/*** MUI Defines ***/

#ifdef _DCC
 extern const char MUIC_Popposition[];
#else
# define MUIC_Popposition "Popposition.mcc"
#endif

#define PoppositionObject MUI_NewObject(MUIC_Popposition



/*** Methods ***/

#define MUIM_Popposition_Close          0x80020080
#define MUIM_Popposition_Open           0x80020081

/*** Method structs ***/

struct MUIP_Popposition_Close {
	ULONG MethodID;
	ULONG Type;
};

struct MUIP_Popposition_Open {
	ULONG MethodID;
};


/*** Special method values ***/

#define MUIV_Popposition_Close_Type_Cancel            0
#define MUIV_Popposition_Close_Type_Ok                1


/*** Special method flags ***/



/*** Attributes ***/

#define MUIA_Popposition_XOffset        0x80020087
#define MUIA_Popposition_XPos           0x8002007d
#define MUIA_Popposition_YOffset        0x80020088
#define MUIA_Popposition_YPos           0x8002007e

/*** Special attribute values ***/



/*** Structures, Flags & Values ***/





#endif /* POPPOSITION_MCC_H */

/* PrMake.rexx 0.10 (16.2.1996) Copyright 1995 kmel, Klaus Melchior */


#pragma pack()
#pragma pack()
