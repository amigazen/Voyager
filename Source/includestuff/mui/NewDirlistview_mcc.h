#pragma pack(2)
#pragma pack(2)
/*

		NewDirlistview.mcc (c) Copyright 1996-97 by JFP, Jean-francois PIK

		Not Registered MUI class!

		NewDirlistview_mcc.c

*/

#ifndef NEWDIRLISTVIEW_MCC_H
#define NEWDIRLISTVIEW_MCC_H

#ifndef LIBRARIES_MUI_H
#pragma pack()
#pragma pack()
#include "libraries/mui.h"
#pragma pack(2)
#pragma pack(2)
#endif

#ifndef DOS_DOSEXTENS_H
#pragma pack()
#pragma pack()
#include <dos/dosextens.h>
#pragma pack(2)
#pragma pack(2)
#endif

/*** MUI Defines ***/

#define MUIC_NewDirlistview   "NewDirlistview.mcc"
#define MUIC_NewDirNlistview  "NewDirNlistview.mcc"

#define NewDirlistviewObject  MUI_NewObject(MUIC_NewDirlistview
#define NewDirNlistviewObject MUI_NewObject(MUIC_NewDirNlistview

#endif /* NEWDIRLISTVIEW_MCC_H */

#pragma pack()
#pragma pack()
