#pragma pack(2)
#pragma pack(2)
/*
**
** $VER: BetterString_mcc.h V11.0 (28-Sep-97)
** Copyright © 1997 Allan Odgaard. All rights reserved.
**
*/

#ifndef   BETTERSTRING_MCC_H
#define   BETTERSTRING_MCC_H

#ifndef   EXEC_TYPES_H
#pragma pack()
#pragma pack()
#include  <exec/types.h>
#pragma pack(2)
#pragma pack(2)
#endif

#define   MUIC_BetterString     "BetterString.mcc"
#define   BetterStringObject    MUI_NewObject(MUIC_BetterString

#define MUIA_BetterString_SelectSize    0xad001001
#define MUIA_BetterString_StayActive    0xad001003
#define MUIM_BetterString_ClearSelected 0xad001004
#define MUIM_BetterString_Insert        0xad001002
#define MUIA_BetterString_Columns       0xad001005

struct MUIP_BetterString_ClearSelected {ULONG MethodID; };
struct MUIP_BetterString_Insert        {ULONG MethodID; STRPTR text; LONG pos; };

#define MUIV_BetterString_Insert_StartOfString  0x00000000
#define MUIV_BetterString_Insert_EndOfString    0xfffffffe
#define MUIV_BetterString_Insert_BufferPos      0xffffffff

#endif /* BETTERSTRING_MCC_H */

#pragma pack()
#pragma pack()
