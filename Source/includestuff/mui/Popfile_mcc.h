#pragma pack(2)
#pragma pack(2)
/*

		Popfile.mcc (c) Copyright 1996-97 by JFP, Jean-francois PIK

		Not Registered MUI class!

		Popfile_mcc.c

*/

#ifndef POPFILE_MCC_H
#define POPFILE_MCC_H

#ifndef LIBRARIES_MUI_H
#pragma pack()
#pragma pack()
#include "libraries/mui.h"
#pragma pack(2)
#pragma pack(2)
#endif

#ifndef NEWDIRLIST_MCC_H
#pragma pack()
#pragma pack()
#include <mui/NewDirList_mcc.h>
#pragma pack(2)
#pragma pack(2)
#endif

#ifndef FILE_MCC_H
#pragma pack()
#pragma pack()
#include <mui/File_mcc.h>
#pragma pack(2)
#pragma pack(2)
#endif

#ifndef MUIFILEREQ_H
#pragma pack()
#pragma pack()
#include <libraries/MUIFilereq.h>
#pragma pack(2)
#pragma pack(2)
#endif

/*** MUI Defines ***/

#define MUIC_Popfile      "Popfile.mcc"

#define PopfileObject     MUI_NewObject(MUIC_Popfile

/*** Methods ***/

/*** Attributes ***/

#define MUIA_Popfile_Title            0x80030090      /* IS. char* */
#define MUIA_Popfile_Active           0x80030091      /* ..G BOOL  */
#define MUIA_Popfile_UseNList         0x80030092      /* I.. BOOL  */
#define MUIA_Popfile_Sleep            0x80030093      /* ISG BOOL  */

#define PopNfileObject    MUI_NewObject(MUIC_Popfile, MUIA_Popfile_UseNList, TRUE

#endif /* POPFILE_MCC_H */

#pragma pack()
#pragma pack()
