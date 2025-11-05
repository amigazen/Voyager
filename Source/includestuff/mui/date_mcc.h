#pragma pack(2)
#pragma pack(2)
/*

		Date.mcc (c) Copyright 1996-97 by JFP, Jean-francois PIK

		Not Registered MUI class!

		Date.c

*/

#ifndef DATE_MCC_H
#define DATE_MCC_H

#ifndef LIBRARIES_MUI_H
#pragma pack()
#pragma pack()
#include "libraries/mui.h"
#pragma pack(2)
#pragma pack(2)
#endif


/*** MUI Defines ***/

#define MUIC_Date  "Date.mcc"
#define MUIC_DateP "Date.mcp"
#define DateObject MUI_NewObject(MUIC_Date

/*** Methods ***/

#define MUIM_Date_AddDays           0x80010070
#define MUIM_Date_SetDefault        0x80010071
#define MUIM_Date_GetString         0x80010072   /* char* */

struct MUIP_Date_AddDays            { long id,val; };
struct MUIP_Date_GetString          { long id,days; };


/*** Attributes ***/

#define MUIA_Date_Date              0x80010060  /* struct ClockData * [ISG] */
#define MUIA_Date_DefaultDays       0x80010061  /* long [ISG] */
#define MUIA_Date_DefaultDate       0x80010062  /* struct ClockDate * [ISG] */

#define MUIA_Date_Days              0x80010063  /* long [ISG] */
#define MUIA_Date_ArrowPosition     0x80010067  /* BOOL [I..] */
#define MUIA_Date_Format            0x80010068  /* long [IS.] */
#define MUIA_Date_String            0x80010069  /* char * [..G] */

/*** Special attribute values ***/

#define MUIV_Date_Today                     -1
#define MUIV_Date_Tomorrow                  -2
#define MUIV_Date_Yesterday                 -3

/** Week date **/
#define MUIV_Date_BeginOfWeek               -4
#define MUIV_Date_EndOfWeek                 -5
#define MUIV_Date_BeginOfNextWeek           -6
#define MUIV_Date_NextWeek                  -7
#define MUIV_Date_EndOfNextWeek             -8
#define MUIV_Date_BeginOfLastWeek           -9
#define MUIV_Date_LastWeek                 -10
#define MUIV_Date_EndOfLastWeek            -11

/** Month Date **/
#define MUIV_Date_BeginOfMonth             -12
#define MUIV_Date_EndOfMonth               -13
#define MUIV_Date_BeginOfNextMonth         -14
#define MUIV_Date_NextMonth                -15
#define MUIV_Date_EndOfNextMonth           -16
#define MUIV_Date_BeginOfLastMonth         -17
#define MUIV_Date_LastMonth                -18
#define MUIV_Date_EndOfLastMonth           -19

/****** arrow position ******/
#define MUIV_Date_ArrowPosition_Right        0 
#define MUIV_Date_ArrowPosition_Left         1 
#define MUIV_Date_ArrowPosition_LeftRight    2

/****** format for date *****/

#define MUIV_Date_Format_MonthDay            0
#define MUIV_Date_Format_DayMonth            1

#endif /* DATE_MCC_H */

#pragma pack()
#pragma pack()
