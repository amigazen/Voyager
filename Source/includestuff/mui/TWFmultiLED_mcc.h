#pragma pack(2)
#pragma pack(2)
//     ___       ___
//   _/  /_______\  \_     ___ ___ __ _                       _ __ ___ ___
//__//  / _______ \  \\___/                                               \___
//_/ | '  \__ __/  ` | \_/        © Copyright 1998, Christopher Page       \__
// \ | |    | |__  | | / \               All Rights Reserved               /
//  >| .    |  _/  . |<   >--- --- -- -                       - -- --- ---<
// / \  \   | |   /  / \ / This file may not be distributed, reproduced or \
// \  \  \_/   \_/  /  / \  altered, in full or in part, without written   /
//  \  \           /  /   \    permission from Christopher Page except    /
// //\  \_________/  /\\ //\       under the conditions given in the     /
//- --\   _______   /-- - --\           package documentation           /-----
//-----\_/       \_/---------\   ___________________________________   /------
//                            \_/                                   \_/
//

#ifndef TWFMULTILED_MCC_H
#define TWFMULTILED_MCC_H

#ifndef LIBRARIES_MUI_H
#pragma pack()
#pragma pack()
#include "libraries/mui.h"
#pragma pack(2)
#pragma pack(2)
#endif


/*** MUI Defines ***/
#define MUIC_TWFmultiLED "TWFmultiLED.mcc"
#define TWFmultiLEDObject MUI_NewObject(MUIC_TWFmultiLED


/*** Attributes ***/
#define MUIA_TWFmultiLED_Colour             0xfebd0001
#define MUIA_TWFmultiLED_Custom             0xfebd0002
#define MUIA_TWFmultiLED_Type               0xfebd0003
#define MUIA_TWFmultiLED_Free               0xfebd0004
#define MUIA_TWFmultiLED_TimeDelay          0xfebd0005   /*** New in V12.4 ***/


/*** Special attribute values ***/
#define MUIV_TWFmultiLED_Colour_Off         0
#define MUIV_TWFmultiLED_Colour_On          1
#define MUIV_TWFmultiLED_Colour_Ok          2
#define MUIV_TWFmultiLED_Colour_Load        3
#define MUIV_TWFmultiLED_Colour_Error       4
#define MUIV_TWFmultiLED_Colour_Panic       5
#define MUIV_TWFmultiLED_Colour_Custom      6
#define MUIV_TWFmultiLED_Colour_Working     7            /*** New in V12.4 ***/
#define MUIV_TWFmultiLED_Colour_Waiting     8            /*** New in V12.4 ***/
#define MUIV_TWFmultiLED_Colour_Cancelled   9            /*** New in V12.4 ***/
#define MUIV_TWFmultiLED_Colour_Stopped     10           /*** New in V12.4 ***/

#define MUIV_TWFmultiLED_Type_Round5        0
#define MUIV_TWFmultiLED_Type_Round11       1
#define MUIV_TWFmultiLED_Type_Square5       2
#define MUIV_TWFmultiLED_Type_Square11      3
#define MUIV_TWFmultiLED_Type_Rect11        4
#define MUIV_TWFmultiLED_Type_Rect15        5
#define MUIV_TWFmultiLED_Type_User          6

#define MUIV_TWFmultiLED_TimeDelay_User     -1           /*** New in V12.4 ***/
#define MUIV_TWFmultiLED_TimeDelay_Off      0            /*** New in V12.4 ***/


/*** Structures, Flags & Values ***/
struct TWFmultiLED_RGB
{
    ULONG Red;
    ULONG Green;
    ULONG Blue;
};

#endif



#pragma pack()
#pragma pack()
