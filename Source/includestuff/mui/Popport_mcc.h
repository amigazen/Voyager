#pragma pack(2)
#pragma pack(2)
/*

		MCC_Popport (c) by kMel, Klaus Melchior

		Registered class of the Magic User Interface.

		Popport_mcc.h

*/


/*** Include stuff ***/

#ifndef POPPORT_MCC_H
#define POPPORT_MCC_H

#ifndef LIBRARIES_MUI_H
#pragma pack()
#pragma pack()
#include "libraries/mui.h"
#pragma pack(2)
#pragma pack(2)
#endif


/*** MUI Defines ***/

#ifdef _DCC
 extern const char MUIC_Popport[];
#else
# define MUIC_Popport "Popport.mcc"
#endif

#define PopportObject MUI_NewObject(MUIC_Popport



/*** Methods ***/


/*** Method structs ***/


/*** Special method values ***/


/*** Special method flags ***/



/*** Attributes ***/

#define MUIA_Popport_ARexxOnly      0x80020098
#define MUIA_Popport_PortPattern    0x80020096
#define MUIA_Popport_TaskPattern    0x80020095
#define MUIA_Popport_Tasks          0x80020097

/*** Special attribute values ***/



/*** Structures, Flags & Values ***/





#endif /* POPPORT_MCC_H */

/* PrMake.rexx 0.10 (16.2.1996) Copyright 1995 kmel, Klaus Melchior */


#pragma pack()
#pragma pack()
