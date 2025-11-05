#pragma pack(2)
#pragma pack(2)
#ifndef CMANAGER_H
#define CMANAGER_H

/*
    $VER: CManager.h 1.7 (14.9.99) ©1999 by Simone Tellini
*/

#ifndef MAKE_ID
#define MAKE_ID(a,b,c,d)    ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#endif


#define CME_GROUP       0
#define CME_USER        1
#define CME_WWW         2
#define CME_FTP         3
#define CME_CHAT        4
#define CME_TELNET      5

#define CME_LIST        253
#define CME_IMAGE       254
#define CME_SECTION     255

struct CMGroup {
        struct CMGroup *Succ;
        struct CMGroup *Prec;
        UBYTE           Type;
        UBYTE           Flags;
        TEXT            Name[80];
        struct MinList  Entries;
        struct MinList  SubGroups;
};

#define GRPF_BOLD       (1 << 0)
#define GRPF_OPEN       (1 << 1)

struct CMUser {
        struct CMUser  *Succ;
        struct CMUser  *Prec;
        UBYTE           Type;
        UBYTE           Flags;
        TEXT            Name[80];       //  FirstName
        TEXT            Address[128];
        TEXT            City[128];
        TEXT            Country[80];
        TEXT            ZIP[20];
        TEXT            Comment[512];
        TEXT            Alias[40];
        TEXT            Phone[40];
        TEXT            Fax[40];
        TEXT            EMail[128];
        TEXT            WWW[256];
        TEXT            FTP[256];
        TEXT            LastName[80];
        TEXT            Mobile[40];
        TEXT            ICQ[20];
        TEXT            Company[40];    //  Business data
        TEXT            BAddress[128];
        TEXT            BCity[128];
        TEXT            BCountry[80];
        TEXT            BZIP[20];
        TEXT            BPhone[40];
        TEXT            BFax[40];
        TEXT            BEMail[128];
        TEXT            BJobTitle[20];
        TEXT            BDepartment[40];
        TEXT            BOffice[40];
        struct CMImage *Image;
        TEXT            PGPUserID[80];
        TEXT            BMobile[40];
};

struct CMWWW {
        struct CMWWW       *Succ;
        struct CMWWW       *Prec;
        UBYTE               Type;
        UBYTE               Flags;
        TEXT                Name[80];
        TEXT                WWW[256];
        TEXT                Comment[512];
        TEXT                WebMaster[80];
        TEXT                EMail[128];
        struct DateStamp    LastModified;
        struct DateStamp    LastVisited;
        TEXT                Alias[40];
};

struct CMFTP {
        struct CMFTP       *Succ;
        struct CMFTP       *Prec;
        UBYTE               Type;
        UBYTE               Flags;
        TEXT                Name[80];
        TEXT                FTP[256];
        TEXT                Comment[512];
        TEXT                Username[60];
        TEXT                Password[60];
        ULONG               Port;
        ULONG               Retries;
        TEXT                Local[256];
        struct DateStamp    LastModified;
        struct DateStamp    LastVisited;
        TEXT                Alias[40];
        ULONG               ExtraFlags;     // lib v10
        UWORD               WindowStyle;    // lib v10
};

// flags
#define FTPF_ADVANCED           (1 << 0)
#define FTPF_QUIET              (1 << 1)
#define FTPF_COMPRESS           (1 << 2)
#define FTPF_ADT                (1 << 3)
#define FTPF_ANONYMOUS          (1 << 4)
#define FTPF_LOCAL              (1 << 5)
#define FTPF_PROXY              (1 << 6)
#define FTPF_SHOW_HIDDEN        (1 << 7)

// extra flags
#define FTPEF_SHOW_FTP_OUTPUT   (1 << 0)
#define FTPEF_NOOPS             (1 << 1)

// window style
#define FTPWS_DUAL_LIST         0
#define FTPWS_SINGLE_LIST       1

struct CMChat {
        struct CMChat  *Succ;
        struct CMChat  *Prec;
        UBYTE           Type;
        UBYTE           Flags;
        TEXT            Channel[128];
        TEXT            Server[128];
        TEXT            Maintainer[80];
        TEXT            Nick[20];
        TEXT            WWW[256];
        TEXT            Comment[512];
        TEXT            Password[60];
};

struct CMTelnet {
        struct CMTelnet    *Succ;
        struct CMTelnet    *Prec;
        UBYTE               Type;
        UBYTE               Flags;
        TEXT                Host[80];
        ULONG               Port;
        TEXT                Login[80];
        TEXT                Password[80];
        TEXT                Comment[256];
};

#define TNETF_SSH       (1 << 0)

struct CMImage {
        struct CMImage *Succ;
        struct CMImage *Prec;
        UBYTE           Type;
        UBYTE           Flags;
        struct BitMap  *BitMap;
        APTR            Colors;
};


/*
 *          Structure to be passed to CM_LoadData(), CM_SaveData(), CM_FreeData()
 */

struct CMDataOld {                          //  obsolete version, DON'T USE ANYMORE
        struct CMGroup *Users;
        struct CMGroup *WWWs;
        struct CMGroup *FTPs;
        struct CMGroup *Chat;
        TEXT            CurrentUser[60];
        TEXT            Path[256];
        BOOL            FreeMe;
};

struct CMData {
        struct CMGroup *Users;
        struct CMGroup *WWWs;
        struct CMGroup *FTPs;
        struct CMGroup *Chat;
        TEXT            CurrentUser[60];    // set by CM_LoadCurrentUserData()
        TEXT            Path[256];          // set by CM_LoadCurrentUserData()
        BOOL            FreeMe;             // if TRUE, CM_FreeData() frees the structure as well
        UWORD           Version;            // version of this structure, you MUST fill it
        ULONG           SizeOf;             // *** PRIVATE ***
        struct CMGroup *Telnet;
};

#define CMD_CURRENT_VERSION     1

/*
 *          Flags for CM_GetEntry()
 */

#define CMGE_USER           (1 << 0)
#define CMGE_WWW            (1 << 1)
#define CMGE_FTP            (1 << 2)
#define CMGE_CHAT           (1 << 3)
#define CMGE_MULTISELECT    (1 << 4)    //  CM_GetEntry returns a struct MinList *
#define CMGE_TELNET         (1 << 5)





/*
 *          CManager PRIVATE stuff
 */

#define ENTRY_NEW       (1 << 7)

enum {
    SCP_WWW = 0, SCP_FTP, SCP_EMAIL,
    SCP_PHONE, SCP_FAX, SCP_IRC, SCP_DOPUSFTP,
    SCP_MOBILE, SCP_BUSINESS_EMAIL, SCP_BUSINESS_FAX,
    SCP_BUSINESS_PHONE, SCP_BUSINESS_MOBILE
};


#define STACK_SWAPPED   (1 << 0)
#define SEND_AREXX      (1 << 1)
#ifndef SAVED
#define SAVED           (1 << 2)
#endif
#define NO_LOGIN        (1 << 3)
#define FIXED_LOGIN     (1 << 4)
#define LOAD_FILE       (1 << 5)

extern struct CMGroup   Users;
extern struct CMGroup   WWWs;
extern struct CMGroup   FTPs;
extern struct CMGroup   Chat;

extern TEXT             PubScreen[];


#endif /* CMANAGER_H */

#pragma pack()
#pragma pack()
