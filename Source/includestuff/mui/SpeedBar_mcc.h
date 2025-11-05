#pragma pack(2)
#pragma pack(2)
#ifndef SPEEDBAR_MCC_H
#define SPEEDBAR_MCC_H

/*********************************************************
**                                                      **
**      SpeedBar.mcc                                    **
**                                                      **
**              ©1999 Simone Tellini                    **
**                                                      **
*********************************************************/

#ifndef LIBRARIES_MUI_H
#pragma pack()
#pragma pack()
#include <libraries/mui.h>
#pragma pack(2)
#pragma pack(2)
#endif

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


#define MUIC_SpeedBar       "SpeedBar.mcc"
#define SpeedBarObject      MUI_NewObject( MUIC_SpeedBar

#define SBTAGBASE           0xF76B00A0


/*************************
**      Methods         **
*************************/

#define MUIM_SpeedBar_AddButton         (SBTAGBASE + 1)
#define MUIM_SpeedBar_AddButtonObject   (SBTAGBASE + 2)
#define MUIM_SpeedBar_AddSpacer         (SBTAGBASE + 3)
#define MUIM_SpeedBar_Clear             (SBTAGBASE + 4)
#define MUIM_SpeedBar_Rebuild           (SBTAGBASE + 5)
#define MUIM_SpeedBar_AddNotify         (SBTAGBASE + 6)     /*  PRIVATE  */
#define MUIM_SpeedBar_GetObject         (SBTAGBASE + 7)
#define MUIM_SpeedBar_DoOnButton        (SBTAGBASE + 8)


/*************************
**      Attributes      **
*************************/

#define MUIA_SpeedBar_Borderless        (SBTAGBASE + 1)     /*  BOOL                IS.. */
#define MUIA_SpeedBar_Images            (SBTAGBASE + 2)     /*  struct MyBrush **   I.G. */
#define MUIA_SpeedBar_SpacerIndex       (SBTAGBASE + 3)     /*  UWORD               I.G. */
#define MUIA_SpeedBar_RaisingFrame      (SBTAGBASE + 4)     /*  BOOL                IS.. */
#define MUIA_SpeedBar_Buttons           (SBTAGBASE + 5)     /*  struct SBButtons *  I... */
#define MUIA_SpeedBar_ViewMode          (SBTAGBASE + 6)     /*  UWORD               ISG. */
#define MUIA_SpeedBar_SameWidth         (SBTAGBASE + 7)     /*  BOOL                I... */
#define MUIA_SpeedBar_Spread            (SBTAGBASE + 8)     /*  BOOL                I... */
#define MUIA_SpeedBar_StripUnderscore   (SBTAGBASE + 9)     /*  BOOL                I... */
#define MUIA_SpeedBar_SmallImages       (SBTAGBASE + 10)    /*  BOOL                IS.. */
#define MUIA_SpeedBar_Sunny             (SBTAGBASE + 11)    /*  BOOL                IS.. */
#define MUIA_SpeedBar_SameHeight        (SBTAGBASE + 12)    /*  BOOL                I... */


/*************************
**      Values          **
*************************/

#define MUIV_SpeedBar_Spacer            ((ULONG)-1)
#define MUIV_SpeedBar_End               ((ULONG)-2) // ends a MUIS_SpeedBar_Button array

#define MUIV_SpeedBar_ButtonFlag_Immediate  (1 << 0)
#define MUIV_SpeedBar_ButtonFlag_Disabled   (1 << 1)
#define MUIV_SpeedBar_ButtonFlag_Selected   (1 << 2)
#define MUIV_SpeedBar_ButtonFlag_Toggle     (1 << 3)

#define MUIV_SpeedBar_ViewMode_TextGfx  0
#define MUIV_SpeedBar_ViewMode_Gfx      1
#define MUIV_SpeedBar_ViewMode_Text     2


/*************************
**      Structures      **
*************************/

struct MUIS_SpeedBar_Button {
        ULONG           Img;        /* image index                                          */
        STRPTR          Text;       /* button label                                         */
        STRPTR          Help;       /* button help                                          */
        UWORD           Flags;
        struct IClass  *Class;      /* easy way of getting a toolbar of subclassed buttons  */
        Object         *Object;     /* filled after init                                    */
};

struct MUIP_SpeedBar_AddButton { ULONG MethodID; struct MUIS_SpeedBar_Button *Button; };
struct MUIP_SpeedBar_AddButtonObject { ULONG MethodID; Object *Button; };
struct MUIP_SpeedBar_AddNotify { ULONG MethodID; Object *Dest; struct MUIP_Notify *Msg; };
struct MUIP_SpeedBar_GetObject { ULONG MethodID; ULONG Object; };
struct MUIP_SpeedBar_DoOnButton { ULONG MethodID; ULONG Button; ULONG Method; /* ...args... */ };


#endif /* SPEEDBAR_MCC_H */

#pragma pack()
#pragma pack()
