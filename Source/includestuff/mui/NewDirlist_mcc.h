#pragma pack(2)
#pragma pack(2)
/*

		NewDirList.mcc (c) Copyright 1996-97 by JFP, Jean-francois PIK

		Not Registered MUI class!

		NewDirList_mcc.c

*/

#ifndef NEWDIRLIST_MCC_H
#define NEWDIRLIST_MCC_H

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

#define MUIC_NewDirlist   "NewDirlist.mcc"
#define MUIC_NewDirNlist  "NewDirNlist.mcc"
#define MUIC_NewDirlistP  "NewDirlist.mcp"
#define NewDirlistObject  MUI_NewObject(MUIC_NewDirlist
#define NewDirNlistObject MUI_NewObject(MUIC_NewDirNlist

/*** Methods ***/

#define MUIM_NewDirlist_ReadVolume      0x80030001
#define MUIM_NewDirlist_DoubleClick     0x80030003
#define MUIM_NewDirlist_SetDefault      0x80030004
#define MUIM_NewDirlist_ReadDir         0x80030005
#define MUIM_NewDirlist_Parent          0x80030006
#define MUIM_NewDirlist_GetPath         0x8003000a
#define MUIM_NewDirlist_VolumePos       0x8003000e
#define MUIM_NewDirlist_GetEntry        0x80030012
#define MUIM_NewDirlist_DragDrop        0x80030013
#define MUIM_NewDirlist_InsertEntry     0x80030014
#define MUIM_NewDirlist_RemoveEntry     0x80030015

struct MUIP_NewDirlist_ReadVolume       { LONG ID;};
struct MUIP_NewDirlist_DoubleClick      { LONG ID, mx,my, active; };
struct MUIP_NewDirlist_SetDefault       { LONG ID;};
struct MUIP_NewDirlist_ReadDir          { LONG ID;};
struct MUIP_NewDirlist_Parent           { LONG ID;};
struct MUIP_NewDirlist_GetPath          { LONG ID,pos; ULONG *store; };
struct MUIP_NewDirlist_VolumePos        { LONG ID;struct VolumeList *vl; };
struct MUIP_NewDirlist_GetEntry         { ULONG ID; long pos; APTR *entry; long *state; };
struct MUIP_NewDirlist_DragReport       { ULONG ID; Object *obj; LONG x; LONG y; LONG update; };
struct MUIP_NewDirlist_InsertEntry      { ULONG ID; char *entry; LONG active; };
struct MUIP_NewDirlist_RemoveEntry      { ULONG ID; long pos; };

/*** Attributes ***/

#define MUIA_NewDirlist_VolumeFormat    0x80030020     /* ISG char* */
#define MUIA_NewDirlist_DirFormat       0x80030021     /* ISG char* */
#define MUIA_NewDirlist_State           0x80030022     /* ISG long  */
#define MUIA_NewDirlist_VolumeType      0x80030023     /* ISG long  */
#define MUIA_NewDirlist_DefaultDir      0x80030024     /* ISG char* */
#define MUIA_NewDirlist_Directory       0x80030025     /* ISG char* */
#define MUIA_NewDirlist_Path            0x80030027     /* .SG char* */
#define MUIA_NewDirlist_Status          0x80030028     /* ..G       */
#define MUIA_NewDirlist_EnterDirs       0x80030029     /* ISG BOOL  */
#define MUIA_NewDirlist_FileSelected    0x8003002a     /* ..G long  */
#define MUIA_NewDirlist_AttachedString  0x8003002b     /* IS. Object* */
#define MUIA_NewDirlist_Active          0x8003002c     /* .SG long  */
#define MUIA_NewDirlist_AcceptPattern   0x8003002d     /* IS. STRPTR            */
#define MUIA_NewDirlist_DrawersOnly     0x8003002e     /* ISG BOOL              */
#define MUIA_NewDirlist_FilesOnly       0x8003002f     /* ISG BOOL              */
#define MUIA_NewDirlist_FilterDrawers   0x80030030     /* ISG BOOL              */
#define MUIA_NewDirlist_FilterHook      0x80030031     /* IS. struct Hook *     */
#define MUIA_NewDirlist_MultiSelDirs    0x80030032     /* isg BOOL              */
#define MUIA_NewDirlist_NumBytes        0x80030033     /* ..g LONG              */
#define MUIA_NewDirlist_NumDrawers      0x80030034     /* ..g LONG              */
#define MUIA_NewDirlist_NumFiles        0x80030035     /* ..g LONG              */
#define MUIA_NewDirlist_RejectIcons     0x80030036     /* isg BOOL              */
#define MUIA_NewDirlist_RejectPattern   0x80030037     /* is. STRPTR            */
#define MUIA_NewDirlist_SortDirs        0x80030038     /* isg LONG              */
#define MUIA_NewDirlist_SortHighLow     0x80030039     /* isg BOOL              */
#define MUIA_NewDirlist_SortType        0x8003003a     /* isg LONG              */
#define MUIA_NewDirlist_HilightDrawers  0x8003003b     /* isg BOOL              */
#define MUIA_NewDirlist_RescanVolumes   0x8003003d     /* isg BOOL              */
#define MUIA_NewDirlist_ActiveDrawers   0x8003003e     /* isg BOOL              */
#define MUIA_NewDirlist_MultiSelect     0x8003003f     /* isg BOOL              */
#define MUIA_NewDirlist_Title           0x80030040     /* isg BOOL              */
#define MUIA_NewDirlist_SaveMode        0x80030041     /* isg BOOL              */
#define MUIA_NewDirlist_DefaultSequence 0x80030043     /* IS. char*             */
#define MUIA_NewDirlist_RemoveHook      0x80030045     /* IS. struct Hook *     */

/*** Special Values for Methods ***/

#define MUIV_NewDirlist_GetPath_Active          -1

#define MUIV_NewDirlist_GetEntry_Active                 -1

  /*** MUIM_NewDirlist_GetEntry returns either a VolumeEntry ptr ***/
  /*** or a FileInfoBlock depending on the context... ***/

struct VolumeEntry
{
  char *ve_Name;
  long ve_PUsed, ve_Size, ve_SFree,ve_SUsed;
};

#define MUIV_NewDirlist_InsertEntry_Active_Inserted     -1

#define MUIV_NewDirlist_RemoveEntry_Active              -1
#define MUIV_NewDirlist_RemoveEntry_Selected            -2

struct MUIS_NewDirlist_RemoveHook_Msg
{
  struct FileInfoBlock *fib;
};

/*** Special Values for Attributes ***/

#define MUIV_NewDirlist_State_Alternate         -1
#define MUIV_NewDirlist_State_Drawer             0
#define MUIV_NewDirlist_State_Volume             1

#define MUIV_NewDirlist_Status_Read              0
#define MUIV_NewDirlist_Status_Invalid           1
#define MUIV_NewDirlist_Status_Valid             2
#define MUIV_NewDirlist_Status_Error             3

#define MUIV_NewDirlist_Directory_Current       -1
#define MUIV_NewDirlist_Directory_PROGDIR       -2

#define MUIV_NewDirlist_Directory_Default       -1

#define MUIV_NewDirlist_SortDirs_First           0
#define MUIV_NewDirlist_SortDirs_Last            1
#define MUIV_NewDirlist_SortDirs_Mix             2

#define MUIV_NewDirlist_SortType_Name            0
#define MUIV_NewDirlist_SortType_Date            1
#define MUIV_NewDirlist_SortType_Size            2

#define MUIV_NewDirlist_VolumeFormat_Default    "MAW=100, P=\033r, P=\033r, P=\033r, P=\033r" /* Image - Name , % used, size , free size, used size */
#define MUIV_NewDirlist_DirFormat_Default       "MAW=95, MAW=5 P=\033r, MIW=0 P=\033r, MIW=0 P=\033r, MIW=0 P=\033r" /* Name  , Image or size, protection, date, time */
#define MUIV_NewDirNlist_VolumeFormat_Default   "W=-1 BAR, W=-1 P=\033r BAR, W=-1 P=\033r BAR, W=-1 P=\033r BAR, W=-1 P=\033r" /* Image - Name | % used | size | free size | used size */
#define MUIV_NewDirNlist_DirFormat_Default      "W=-1 BAR, W=-1 P=\033r BAR, W=-1 P=\033r BAR, W=-1 P=\033r BAR, W=-1 P=\033r BAR, W=-1" /* Name  | Image or size| protection | date | time | Comment */

#define MUIV_NewDirlist_VolumeType_All          LDF_ALL
#define MUIV_NewDirlist_VolumeType_Devices      LDF_DEVICES
#define MUIV_NewDirlist_VolumeType_Volumes      LDF_VOLUMES
#define MUIV_NewDirlist_VolumeType_Assigns      LDF_ASSIGNS

/* volume type can be add -> (LDF_VOLUMES|LDF_ASSIGNS) */

#endif /* NEWDIRLIST_MCC_H */

#pragma pack()
#pragma pack()
