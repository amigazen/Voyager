#pragma pack(2)
#pragma pack(2)
#ifndef CMANAGER_MCC_H
#define CMANAGER_MCC_H

/*********************************************************
**                                                      **
**      CManager.mcc                                    **
**                                                      **
**              ©1999-2000 Simone Tellini               **
**                                                      **
*********************************************************/


/// Support stuff
#ifndef REG
#ifdef _DCC
#define REG(x) __ ## x
#else
#define REG(x) register __ ## x
#endif
#endif

#ifndef ASM
#if defined __MAXON__ || defined __STORM__ || defined _DCC
#define ASM
#else
#define ASM __asm
#endif
#endif

#ifndef SAVEDS
#ifdef __MAXON__
#define SAVEDS
#endif
#if defined __STORM__ || defined __SASC
#define SAVEDS __saveds
#endif
#if defined _GCC || defined _DCC
#define SAVEDS __geta4
#endif
#endif


#ifndef MAKE_ID
#define MAKE_ID(a,b,c,d)    ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#endif

#ifndef MIN
#define MIN(a,b)    (( a < b ) ? ( a ) : ( b ))
#endif

#ifndef MAX
#define MAX(a,b)    (( a > b ) ? ( a ) : ( b ))
#endif

#ifndef ABS
#define ABS(a) (((a) > 0) ? (a) : -(a))
#endif
///


#define MUIC_CManager   "CManager.mcc"
#define CManagerObject  MUI_NewObject( MUIC_CManager


/*************************
**      Methods         **
*************************/

#define MUIM_CManager_LoadData          0xF76B0011
#define MUIM_CManager_SaveData          0xF76B0016
#define MUIM_CManager_Sort              0xF76B0017
#define MUIM_CManager_SetGroup          0xF76B0012  /* PRIVATE */
#define MUIM_CManager_HandleNotify      0xF76B0013  /* PRIVATE */
#define MUIM_CManager_ChangeSort        0xF76B0014  /* PRIVATE */
#define MUIM_CManager_FreeData          0xF76B0015  /* PRIVATE */
#define MUIM_CManager_GetGroup          0xF76B0018  /* PRIVATE */
#define MUIM_CManager_DisposeObj        0xF76B0019  /* PRIVATE */
#define MUIM_CManager_Arrow             0xF76B001A  /* PRIVATE */
#define MUIM_CManager_AddGroup          0xF76B001B  /* PRIVATE */
#define MUIM_CManager_RemGroup          0xF76B001C  /* PRIVATE */
#define MUIM_CManager_AddItem           0xF76B001D  /* PRIVATE */
#define MUIM_CManager_GrabLists         0xF76B001E
#define MUIM_CManager_ReinsertLists     0xF76B001F
#define MUIM_CManager_EditSelected      0xF76B0020
#define MUIM_CManager_OnDoubleClick     0xF76B0021
#define MUIM_CManager_LoadPrefs         0xF76B0022  /* PRIVATE */
#define MUIM_CManager_AddEntry          0xF76B0023
#define MUIM_CManager_Import            0xF76B0024
#define MUIM_CManager_Login             0xF76B0025  /* PRIVATE */
#define MUIM_CManager_Export            0xF76B0026  /* v12 */
#define MUIM_CManager_Search            0xF76B0027  /* v12 */
#define MUIM_CManager_Cleanup           0xF76B0028  /* v14 */


#define MUIV_CManager_AddEntry_CurrentGroup (1 << 0)

// Import types
#define MUIV_CManager_Import_Voyager        0
#define MUIV_CManager_Import_IB             1
#define MUIV_CManager_Import_AWeb           2
#define MUIV_CManager_Import_Users_CSV      3
// flags
#define MUIV_CManager_Import_Filter         (1 << 0) /* avoid duplicate entries */

// Export types
#define MUIV_CManager_Export_HTML_URLs      0       /* export WWW/FTP records   */
#define MUIV_CManager_Export_HTML_WWW       1       /* export WWW records       */
#define MUIV_CManager_Export_HTML_FTP       2       /* export FTP records       */
#define MUIV_CManager_Export_CSV_Users      3


/*************************
**      Attributes      **
*************************/

#define MUIA_CManager_ListObject        0xF76B0010  /* ..G.     Object *        */
#define MUIA_CManager_Changed           0xF76B0012  /* .SGN     BOOL            */
#define MUIA_CManager_Path              0xF76B0013  /* ..G.     STRPTR          */
#define MUIA_CManager_NoMenu            0xF76B0014  /* IS..     BOOL            */
#define MUIA_CManager_HideUsers         0xF76B0015  /* I...     BOOL            */
#define MUIA_CManager_HideWWW           0xF76B0016  /* I...     BOOL            */
#define MUIA_CManager_HideFTP           0xF76B0017  /* I...     BOOL            */
#define MUIA_CManager_HideChat          0xF76B0018  /* I...     BOOL            */
#define MUIA_CManager_CMData            0xF76B0019  /* ..G.     struct CMData * */
#define MUIA_CManager_AppDoubleClick    0xF76B001A  /* IS..     struct Hook *   */
#define MUIA_CManager_HideTelnet        0xF76B001B  /* I...     BOOL            */
#define MUIA_CManager_TreeObject        0xF76B001C  /* ..G.     Object *        */


/*************************
**      Structures      **
*************************/

struct MUIP_CManager_LoadData   { ULONG MethodID; STRPTR User; STRPTR Path; };
								// User == NULL -> Current, User & Path are mutually exclusive
struct MUIP_CManager_SaveData   { ULONG MethodID; ULONG Ask; STRPTR Path; };
								// Ask for path, use the provided one or save in the user's db
struct MUIP_CManager_AddEntry   { ULONG MethodID; struct CMUser *Entry; ULONG Flags; };
struct MUIP_CManager_SetGroup   { ULONG MethodID; struct MUIS_Listtree_TreeNode *Node; }; /* PRIVATE */
struct MUIP_CManager_ChangeSort { ULONG MethodID; ULONG Column; };  /* PRIVATE */
struct MUIP_CManager_DisposeObj { ULONG MethodID; APTR Object; };  /* PRIVATE */
struct MUIP_CManager_Arrow      { ULONG MethodID; ULONG Direction; }; /* PRIVATE */
struct MUIP_CManager_LoadPrefs  { ULONG MethodID; STRPTR File; }; /* PRIVATE */
struct MUIP_CManager_Import     { ULONG MethodID; ULONG Type; STRPTR File; ULONG Flags; };
struct MUIP_CManager_Export     { ULONG MethodID; ULONG Type; STRPTR File; ULONG Flags; };


#endif /* CMANAGER_MCC_H */

#pragma pack()
#pragma pack()
