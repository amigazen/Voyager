/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_IDENTIFY_H
#define _PPCINLINE_IDENTIFY_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef IDENTIFY_BASE_NAME
#define IDENTIFY_BASE_NAME IdentifyBase
#endif /* !IDENTIFY_BASE_NAME */

#define IdAlert(ID, TagList) \
	LP2(0x2a, LONG, IdAlert, ULONG, ID, d0, struct TagItem *, TagList, a0, \
	, IDENTIFY_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define IdAlertTags(a0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; IdAlert((a0), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define IdEstimateFormatSize(String, Tags) \
	LP2(0x48, ULONG, IdEstimateFormatSize, STRPTR, String, a0, struct TagItem *, Tags, a1, \
	, IDENTIFY_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define IdEstimateFormatSizeTags(a0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; IdEstimateFormatSize((a0), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define IdExpansion(TagList) \
	LP1(0x1e, LONG, IdExpansion, struct TagItem *, TagList, a0, \
	, IDENTIFY_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define IdExpansionTags(...) \
	({ULONG _tags[] = { __VA_ARGS__ }; IdExpansion((struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define IdFormatString(String, Buffer, Length, Tags) \
	LP4(0x42, ULONG, IdFormatString, STRPTR, String, a0, STRPTR, Buffer, a1, ULONG, Length, d0, struct TagItem *, Tags, a2, \
	, IDENTIFY_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define IdFormatStringTags(a0, a1, a2, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; IdFormatString((a0), (a1), (a2), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define IdFunction(LibName, Offset, TagList) \
	LP3(0x30, LONG, IdFunction, STRPTR, LibName, a0, LONG, Offset, d0, struct TagItem *, TagList, a1, \
	, IDENTIFY_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define IdFunctionTags(a0, a1, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; IdFunction((a0), (a1), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define IdHardware(Type, TagList) \
	LP2(0x24, STRPTR, IdHardware, ULONG, Type, d0, struct TagItem *, TagList, a0, \
	, IDENTIFY_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define IdHardwareTags(a0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; IdHardware((a0), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define IdHardwareNum(Type, TagList) \
	LP2(0x36, ULONG, IdHardwareNum, ULONG, Type, d0, struct TagItem *, TagList, a0, \
	, IDENTIFY_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define IdHardwareNumTags(a0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; IdHardwareNum((a0), (struct TagItem *)_tags);})
#endif /* !NO_PPCINLINE_STDARG */

#define IdHardwareUpdate() \
	LP0NR(0x3c, IdHardwareUpdate, \
	, IDENTIFY_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#endif /* !_PPCINLINE_IDENTIFY_H */
