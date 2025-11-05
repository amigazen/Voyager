/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_CMANAGER_H
#define _PPCINLINE_CMANAGER_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef CMANAGER_BASE_NAME
#define CMANAGER_BASE_NAME CManagerBase
#endif /* !CMANAGER_BASE_NAME */

#define CM_AddEntry(entry) \
	LP1(0x60, BOOL, CM_AddEntry, APTR, entry, a0, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_AllocCMData() \
	LP0(0x84, struct CMData   *, CM_AllocCMData, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_AllocEntry(type) \
	LP1(0x3c, APTR, CM_AllocEntry, ULONG, type, d0, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_CreateBitMap(width, height, depth, flags, friend) \
	LP5(0x54, struct BitMap   *, CM_CreateBitMap, ULONG, width, d0, ULONG, height, d1, ULONG, depth, d2, ULONG, flags, d3, struct BitMap *, friend, a0, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_DeleteBitMap(bitmap) \
	LP1NR(0x5a, CM_DeleteBitMap, struct BitMap *, bitmap, a0, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_FreeData(data) \
	LP1NR(0x7e, CM_FreeData, struct CMData *, data, a0, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_FreeDataOld(data) \
	LP1NR(0x30, CM_FreeDataOld, struct CMDataOld *, data, a0, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_FreeEntry(entry) \
	LP1NR(0x42, CM_FreeEntry, APTR, entry, a0, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_FreeHandle(handle, close) \
	LP2NR(0x48, CM_FreeHandle, APTR, handle, a0, BOOL, close, d0, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_FreeList(list) \
	LP1NR(0x66, CM_FreeList, struct MinList *, list, a0, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_GetEntry(handle, flags) \
	LP2(0x4e, APTR, CM_GetEntry, APTR, handle, a0, ULONG, flags, d0, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_GetParent(list, current) \
	LP2(0x36, struct CMGroup  *, CM_GetParent, struct CMGroup *, list, a0, struct CMGroup *, current, a1, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_GetString(id) \
	LP1(0x8a, STRPTR, CM_GetString, ULONG, id, d0, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_LoadCurrentUserData(askuser) \
	LP1(0x6c, struct CMData   *, CM_LoadCurrentUserData, BOOL, askuser, d0, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_LoadData(prefs, data, user) \
	LP3(0x72, BOOL, CM_LoadData, STRPTR, prefs, a0, struct CMData *, data, a1, STRPTR, user, a2, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_LoadDataOld(prefs, data, user) \
	LP3(0x24, BOOL, CM_LoadDataOld, STRPTR, prefs, a0, struct CMDataOld *, data, a1, STRPTR, user, a2, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_SaveData(prefs, data, user) \
	LP3NR(0x78, CM_SaveData, STRPTR, prefs, a0, struct CMData *, data, a1, STRPTR, user, a2, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_SaveDataOld(prefs, data, user) \
	LP3NR(0x2a, CM_SaveDataOld, STRPTR, prefs, a0, struct CMDataOld *, data, a1, STRPTR, user, a2, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define CM_StartManager(file, pubscreen) \
	LP2(0x1e, APTR, CM_StartManager, STRPTR, file, a0, STRPTR, pubscreen, a1, \
	, CMANAGER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#endif /* !_PPCINLINE_CMANAGER_H */
