#pragma pack(2)
#pragma pack(2)
/*

		MCC_Busy (c) by kMel, Klaus Melchior

		Registered class of the Magic User Interface.

		Busy_mcc.h

*/


/*** Include stuff ***/

#ifndef BUSY_MCC_H
#define BUSY_MCC_H

#ifndef LIBRARIES_MUI_H
#pragma pack()
#pragma pack()
#include "libraries/mui.h"
#pragma pack(2)
#pragma pack(2)
#endif


/*** MUI Defines ***/

#define MUIC_Busy "Busy.mcc"
#define MUIC_BusyP "Busy.mcp"
#define BusyObject MUI_NewObject(MUIC_Busy

#define BusyBar\
	BusyObject,\
		GaugeFrame,\
		MUIA_Background, MUII_BACKGROUND,\
		End



/*** Methods ***/

#define MUIM_Busy_Data           0x80020041
#define MUIM_Busy_Move           0x80020001
#define MUIM_Busy_Ticks          0x80020044

/*** Method structs ***/

struct MUIP_Busy_Data {
	ULONG MethodID;
	ULONG ID;
	ULONG Set;
};

struct MUIP_Busy_Move {
	ULONG MethodID;
};

struct MUIP_Busy_Ticks {
	ULONG MethodID;
	ULONG OK;
};


/*** Special method values ***/

#define MUIV_Busy_Data_VersInfo                1
#define MUIV_Busy_Data_Sample                  2


/*** Special method flags ***/



/*** Attributes ***/

#define MUIA_Busy_Fades          0x80020046
#define MUIA_Busy_Number         0x80020048
#define MUIA_Busy_PenSpec1       0x80020003
#define MUIA_Busy_PenSpec2       0x80020004
#define MUIA_Busy_Position       0x80020002
#define MUIA_Busy_Refresh        0x80020045
#define MUIA_Busy_Size           0x80020005
#define MUIA_Busy_Style          0x80020047

/*** Special attribute values ***/

#define MUIV_Busy_Refresh_Flag         0x8000



/*** Structures, Flags & Values ***/





/*** Configs ***/

#define MUICFG_Busy_Fades          0x80020009
#define MUICFG_Busy_Number         0x80020008
#define MUICFG_Busy_PenSpec1       0x80020001
#define MUICFG_Busy_PenSpec2       0x80020002
#define MUICFG_Busy_Size           0x80020004
#define MUICFG_Busy_Style          0x80020003


#endif /* BUSY_MCC_H */

/* PrMake.rexx 0.9 (26.12.1995) Copyright 1995 kmel, Klaus Melchior */


#pragma pack()
#pragma pack()
