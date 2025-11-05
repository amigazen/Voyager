#ifndef  CLIB_IDENTIFY_PROTOS_H
#define  CLIB_IDENTIFY_PROTOS_H

struct TagItem;


/*
**      $VER: identify_protos.h 11.0 (23.4.99)
**
**      (C) Copyright 1996-1999 Richard Körber
**          All Rights Reserved
*/
LONG IdAlert(ULONG, struct TagItem *);
LONG IdAlertTags(ULONG, ULONG,...);
ULONG IdEstimateFormatSize(STRPTR, struct TagItem *);
ULONG IdEstimateFormatSizeTags(STRPTR, ULONG,...);
LONG IdExpansion(struct TagItem *);
LONG IdExpansionTags(ULONG,...);
ULONG IdFormatString(STRPTR, STRPTR, ULONG, struct TagItem *);
ULONG IdFormatStringTags(STRPTR, STRPTR, ULONG, ...);
LONG IdFunction(STRPTR, LONG, struct TagItem *);
LONG IdFunctionTags(STRPTR, LONG, ULONG,...);
STRPTR IdHardware(ULONG, struct TagItem *);
STRPTR IdHardwareTags(ULONG, ULONG,...);
ULONG IdHardwareNum(ULONG, struct TagItem *);
ULONG IdHardwareNumTags(ULONG, ULONG,...);
void IdHardwareUpdate(void);

#endif
