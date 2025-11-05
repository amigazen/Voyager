#pragma pack(2)
#pragma pack(2)
#ifndef SPEEDBUTTON_MCC_H
#define SPEEDBUTTON_MCC_H

/*********************************************************
**                                                      **
**      SpeedButton.mcc                                 **
**                                                      **
**              ©1999 Simone Tellini                    **
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


#define MUIC_SpeedButton    "SpeedButton.mcc"
#define SpeedButtonObject   MUI_NewObject( MUIC_SpeedButton

#define BTTAGBASE           0xF76B0100


/*************************
**      Methods         **
*************************/

#define MUIM_SpeedButton_Update         (BTTAGBASE + 1)     /*  PRIVATE  */


/*************************
**      Attributes      **
*************************/

#define MUIA_SpeedButton_Borderless     (BTTAGBASE + 1)     /*  BOOL                I... */
#define MUIA_SpeedButton_Image          (BTTAGBASE + 2)     /*  struct MyBrush  *   I... */
#define MUIA_SpeedButton_Label          (BTTAGBASE + 3)     /*  STRPTR              I... */
#define MUIA_SpeedButton_ViewMode       (BTTAGBASE + 4)     /*  ULONG               IS.. */
#define MUIA_SpeedButton_Raising        (BTTAGBASE + 5)     /*  BOOL                IS.. */
#define MUIA_SpeedButton_MinWidth       (BTTAGBASE + 6)     /*  BOOL                I.G. */
#define MUIA_SpeedButton_NoClick        (BTTAGBASE + 7)     /*  BOOL                I... */
#define MUIA_SpeedButton_SpeedBar       (BTTAGBASE + 8)     /*  Object *            ISG. */
#define MUIA_SpeedButton_QuietNotify    (BTTAGBASE + 9)     /*  BOOL                .S.. */ /* PRIVATE */
#define MUIA_SpeedButton_ToggleMode     (BTTAGBASE + 10)    /*  BOOL                I... */
#define MUIA_SpeedButton_ShowMe         (BTTAGBASE + 11)    /*  BOOL                ..G. */ /* PRIVATE */
#define MUIA_SpeedButton_ImmediateMode  (BTTAGBASE + 12)    /*  BOOL                I... */
#define MUIA_SpeedButton_StripUnderscore (BTTAGBASE + 13)   /*  BOOL                I... */
#define MUIA_SpeedButton_SmallImage     (BTTAGBASE + 14)    /*  BOOL                IS.. */
#define MUIA_SpeedButton_Sunny          (BTTAGBASE + 15)    /*  BOOL                I... */
#define MUIA_SpeedButton_MinHeight      (BTTAGBASE + 16)    /*  BOOL                I.G. */


/*************************
**      Values          **
*************************/

#define SB_MAXLABELLEN                      40

#define MUIV_SpeedButton_ViewMode_TextGfx   0
#define MUIV_SpeedButton_ViewMode_Gfx       1
#define MUIV_SpeedButton_ViewMode_Text      2


/*************************
**      Structures      **
*************************/

struct MUIP_SpeedButton_Update { ULONG MethodID; ULONG Selected; };


#endif /* SPEEDBUTTON_MCC_H */

#pragma pack()
#pragma pack()
