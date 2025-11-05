#pragma pack(2)
#pragma pack(2)
/*

		File.mcc (c) Copyright 1996-97 by JFP, Jean-francois PIK

		Not Registered MUI class!

		File_mcc.c

*/

#ifndef FILE_MCC_H
#define FILE_MCC_H

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
#include "NewDirlist_mcc.h"
#pragma pack(2)
#pragma pack(2)
#endif

#ifndef NEWDIRLISTVIEW_MCC_H
#pragma pack()
#pragma pack()
#include "NewDirlistview_mcc.h"
#pragma pack(2)
#pragma pack(2)
#endif

/*#ifndef LED_MCC_H
#pragma pack()
#pragma pack()
#include "Led_mcc.h"
#pragma pack(2)
#pragma pack(2)
#endif*/

/*** MUI Defines ***/

#define MUIC_File         "File.mcc"

#define FileObject        MUI_NewObject(MUIC_File

struct MUIS_FilesList
{
  struct MUIS_FilesList *NextFile;
  char *path;
};
/*** Methods ***/

#define MUIM_File_Selected              0x80030070
#define MUIM_File_SelectedPattern       0x80030071
#define MUIM_File_ListToDirectory       0x80030072
#define MUIM_File_DirectoryToList       0x80030073
#define MUIM_File_Pattern               0x80030074
#define MUIM_File_FreeFilesList         0x80030075
#define MUIM_File_GetFilesList          0x80030076
#define MUIM_File_NumSelected           0x80030077
#define MUIM_File_DoLed                 0x80030078

struct  MUIP_File_Selected              { LONG id; };
struct  MUIP_File_SelectedPattern       { LONG id; APTR obj; };
struct  MUIP_File_ListToDirectory       { LONG id; APTR obj; };
struct  MUIP_File_DirectoryToList       { LONG id; APTR obj; };
struct  MUIP_File_Pattern               { LONG id; };
struct  MUIP_File_FreeFilesList         { LONG id; struct MUIS_FilesList *FL; };
struct  MUIP_File_GetFilesList          { LONG id; struct MUIS_FilesList **FL; };
struct  MUIP_File_NumSelected           { LONG id; LONG num; };
struct  MUIP_File_DoLed                 { LONG id, test; };

/*** Attributes ***/

#define MUIA_File_MultiSelect           0x80030080      /* I.. BOOL                   */
#define MUIA_File_FilesList             0x80030082      /* ..G struct MUIS_FileList*  */
#define MUIA_File_PatternObject         0x80030083      /* I.. BOOL                   */
#define MUIA_File_InitialPattern        0x80030084      /* I.. char*   (OBSOLETE)     */
#define MUIA_File_OkText                0x80030085      /* i.. char*                  */
#define MUIA_File_UseLed                0x80030086      /* i.. BOOL                   */
#define MUIA_File_Pattern               0x80030087      /* ISG char*                  */
#define MUIA_File_NumSelectedFiles      0x80030088      /* ..G long                   */
#define MUIA_File_UseNList              0x80030089      /* I.. BOOL                   */
#define MUIA_File_File                  0x8003008b      /* ISGN char*                 */
#define MUIA_File_Title                 0x8003008c      /* ISG  char*                 */
#define MUIA_File_ListObject            0x8003008d      /* ..G Object*                */

#define MUIA_File_DrawerOnly            MUIA_NewDirlist_DrawersOnly
#define MUIA_File_FileOnly              MUIA_NewDirlist_FilesOnly
#define MUIA_File_SaveMode              MUIA_NewDirlist_SaveMode
#define MUIA_File_Close                 MUIA_NewDirlist_FileSelected  /* ..GN LONG                  */

/*** Special Values for Methods ***/

#define MUIV_File_NumSelected_All         -1
#define MUIV_File_NumSelected_None         0

/*** Fileobject using a NList.mcc instead of List.mui ***/

#define NFileObject       MUI_NewObject(MUIC_File, MUIA_File_UseNList, TRUE

#endif /* FILE_MCC_H */

#pragma pack()
#pragma pack()
