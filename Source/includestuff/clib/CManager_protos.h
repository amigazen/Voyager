#ifndef CMANAGER_PROTOS_H
#define CMANAGER_PROTOS_H

struct MinList;
struct CMDataOld;
struct CMData;
struct BitMap;
struct CMGroup;


struct CMGroup  *CM_GetParent( struct CMGroup *, struct CMGroup * );
BOOL             CM_LoadData( STRPTR, struct CMData *, STRPTR );
void             CM_SaveData( STRPTR, struct CMData *, STRPTR );
void             CM_FreeData( struct CMData * );
APTR             CM_StartManager( STRPTR, STRPTR );
void             CM_FreeHandle( APTR, BOOL );
APTR             CM_AllocEntry( ULONG );
void             CM_FreeEntry( APTR );
APTR             CM_GetEntry( APTR, ULONG );
struct BitMap   *CM_CreateBitMap( ULONG, ULONG, ULONG, ULONG, struct BitMap * );
void             CM_DeleteBitMap( struct BitMap * );
BOOL             CM_AddEntry( APTR );
void             CM_FreeList( struct MinList * );
struct CMData   *CM_LoadCurrentUserData( BOOL );
BOOL             CM_LoadDataOld( STRPTR, struct CMDataOld *, STRPTR );
void             CM_SaveDataOld( STRPTR, struct CMDataOld *, STRPTR );
void             CM_FreeDataOld( struct CMDataOld * );
struct CMData   *CM_AllocCMData( void );
STRPTR           CM_GetString( ULONG ID );


#endif /* CMANAGER_PROTOS_H */
