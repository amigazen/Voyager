/**************************************************************************

  =======================
  The Voyager Web Browser
  =======================

  Copyright (C) 1995-2001 by
   Oliver Wagner <owagner@vapor.com>
   All Rights Reserved

  Parts Copyright (C) by
   David Gerber <zapek@vapor.com>
   Jon Bright <jon@siliconcircus.com>
   Matt Sealey <neko@vapor.com>

**************************************************************************/


#ifndef VOYAGER_NETWORK_H
#define VOYAGER_NETWORK_H

/*
 * $Id: network.h,v 1.61 2003/11/06 16:38:53 zapek Exp $
 */

#define CLIB_USERGROUP_PROTOS_H // to avoid name clashes
#define CLIB_SOCKET_INLINES_H

#if USE_NET

#ifdef MBX

#define V_DOES_NOT_WANT_DEBUG 1
#include <modules/net/externs.h>

#else /* !MBX */

#include <sys/types.h>

#include <netdb.h>
#include <netinet/in.h>
#include <amitcp/socketbasetags.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#if defined( __MORPHOS__ ) || defined( __SASC )
#include <proto/socket.h>
#endif /* __MORPHOS__ */

#endif /* !MBX */

#endif /* USE_NET */

#if USE_MIAMI
#include <proto/miami.h>
#endif /* USE_MIAMI */

#include "vssl.h"
#if USE_MIAMI
#include <miami_ssl.h>
#endif /* USE_MIAMI */

#include "urlparser.h"
#include "dns.h"

#define MULTIPART_SEP "X-V3MultiPart2-X"

#if USE_CONNECT_PROC

struct connectmsg {
	struct Message m;
	struct sockaddr_in addr;
	int sock;
	int rc;
	int done;
};

#endif /* USE_CONNECT_PROC */

struct unode {
	struct MinNode n;
	char *url;
	char *referer;
	char *urlcopy; // working copy of url string
	char *urlcopy2; // second copy, with postid removed
	APTR pool; // pool this URLn is allocate from (including buffers)
	struct parsedurl purl;
	char *redirecturl; // has been redirected
	char *tofilename; // were we did save the data to, if not loaded to mem
	int sock, sock_pasv;
	int sockstate;
	int sockstategot;
	int state;
	int retr_mode; // 0 = http, 1 = ftp, 2 = file, 3 = internal
	int protocolstate; // protocol state (specific to every subsystem: FTP, HTTP, ... )
	int errorcode; // 0 == ok, > 0 = network, < 0 = internal
	int httpcode;
	time_t lastdata, beginxfer;
	int postid;     // if posting data, ID of posting
#if USE_NET
	struct dnsmsg dnsmsg;
#endif /* USE_NET */
	char errorstring[ 256 ];
	struct MinList clients;
	struct MinList headers; // header lines
	char server [ 128 ]; // name and version of the httpd
	char mimetype[ 128 ];
	char authrealm[ 128 ];
	char authdata[ 128 ];
	int useauth; // authentication counter
	APTR membuff;
	int membuffsize;
	int membuffptr;
	int docptr;
	int doclen;
	int chunksize; // size of current chunk
	int informptr; // when did we last send an inform
	time_t cachedate; // verify cache contents
	time_t cacheexpires; // cached object expires when?
	time_t lastmodified; // when object was last modified
	int ledobjnum;
	APTR docinfowin;   // Document Information window
	struct dnscachenode *dcn;
	int port;         // port to connect to...
#if USE_SSL
	APTR sslh;        // handle for SSL connection
	STRPTR sslcipher;   // SSL cipher informations (string)
	STRPTR sslversion; // SSL version
	STRPTR sslpeercert; // SSL certificate from the peer (string)
#endif /* USE_SSL */
#if USE_DOS
	FileLock_p l;           // lock held on cache contents (for OpenFromLock)
#endif /* USE_DOS */
#if USE_NETFILE
	FileNo fromfile;    // if reading from file...
#endif

#if USE_CONNECT_PROC
	struct connectmsg cmsg;
	int cmsg_pending;
#endif
#if USE_BLOCKING_CONN
	int connect_rc;
#endif

	UBYTE nocache;
	UBYTE stalled;
	UBYTE doinform;
	UBYTE reload;

	UBYTE viaproxy;
	UBYTE ssl;
	UBYTE tomem;    // 1 = read to mem, 2 = reading to mem
	UBYTE chunked;  // reading a chunk

	UBYTE keepalive; // keep connection alive after transfer
	UBYTE wasparked;
	UBYTE trying11; // TRUE if tried an 1.1 request
	UBYTE keepinmem; // keep in memory as cached object

	UBYTE newclientdestmode;
	UBYTE fromcache; // was fetched from on-disk cache
	UBYTE lineread; // line read mode active
	BYTE linereadstate;

	UBYTE net_abort;
	UBYTE ftp_got_quick_done;

	UBYTE pad[ 2 ];

	char *linereadbuffer;
	int linereadbufferptr, linereadadvanceptr;

	/* resume */
	int range; // range the server accepts
	ULONG offset; /* starting offset of the unode. if a client wants a higher offset, defer writing till reached. if it wants a lower one, defer writing then restart */

	struct MinList replybufferlist; // buffered reply lines (mostly for FTP)
	char *ftp_pwd;
	ULONG ftp_pasvip, ftp_pasvport;
	UWORD ftp_filecount, ftp_dircount;
	ULONG ftp_totalfilesize;

	ULONG timeout; /* socket timeout */
};

struct nstream {
	struct Message m;
	char *url;
	char *fullurl;
	char *referer;
	char *filename;
	struct unode *un;
	struct AsyncFile *tofile;
	APTR informobj;
	APTR gaugeobj, statusobj;
	int filestate;  // 0 = ok, otherwise DOS error code
	UBYTE removeme;
	UBYTE informstate; // 1 = client has been informed of document type
	BYTE destmode;
	ULONG timeout; /* socket timeout */
	ULONG offset; /* offset wanted by the nstream */
	ULONG flags;
};


/* destmode */
#define DM_MEMORY 1
#define DM_FILE 2

/* URL Network Stream states */
enum {
	UNS_SETUP,
	UNS_LOOKINGUP,
	UNS_WAITINGFORNET,
	UNS_CONNECTING,
	UNS_CONNECTED,
	UNS_WAITING,
	UNS_FAILED,
	UNS_READING,
	UNS_DONE
};

#define SSW_SR 1
#define SSW_SW 2
#define SSW_PSR 8
#define SSW_PSW 16

/* retr_mode of unode */
enum {
	RM_HTTP,
	RM_FTP,
	RM_FILE,
	RM_INTERNAL
};

#define INTERNALDOC "<HR>\n<ADDRESS>Generated internally by Voyager/" VERSIONSTRING "</ADDRESS>\n</BODY>\n</HTML>\n"

extern time_t now; // set on every cycle

struct rbline {
	struct MinNode n;
	char *data;
};

struct header {
	struct MinNode h;
	char *header;
	char *data;
};

extern int netopen;
extern char proxyauth[ 128 ];
extern int useproxyauth;

extern struct SignalSemaphore netpoolsem;
extern struct Library *VSSLBase;
extern struct Library *MiamiSSLBase;
extern APTR ssl_ctx;
extern struct List sslcertlist;

/*
 * Protos shared between V and the imgdecoders
 */
#include "network_callback.h"

/* protos */
void makehtmlerror( struct unode *un );
void un_netclose( struct unode *un );
int uns_write( struct unode *un, STRPTR data, int len );
void pushreplybuffer( struct unode *un, char *data );
STRPTR unstrdup( struct unode *un, STRPTR str );
void makeneterror( struct unode *un, char *str, int err );
void STDARGS pushfmt( struct unode *un, char *fmt, ... );
void pushstring( struct unode *un, char *string );
int uns_pread( struct unode *un, APTR buffer, int maxlen );
void pushdata( struct unode *un, char *data, int l );
void un_netpark( struct unode *un );
void un_doprotocol( struct unode *un );
void setblocking( int sock, int mode );
APTR unalloc( struct unode *un, int size );
char *ungetheaderdata( struct unode *un, char *header );
int allocdata( struct unode *un, int l );
int uns_read( struct unode *un, char *buffer, int maxlen );
void un_closedestfiles( struct unode *un );
void un_delete( struct unode *un );
int openssl( void );
APTR nalloc( int size );

int nets_getdocrealptr( struct nstream *ns );
void nets_setdocmem( struct nstream *ns, APTR mem, int len );
char * nets_referer( struct nstream *ns );
char *nets_mimeextension( struct nstream *ns );

char * nets_mimetype( struct nstream *ns );
int nets_getdoclen( struct nstream *ns );
void nets_settofile( struct nstream *ns, STRPTR filename, ULONG offset );
void nets_abort( struct nstream *ns );
void nets_setiobj( struct nstream *ns, APTR gauge, APTR txt );
int nets_flushmem( void );
int nets_issecure( struct nstream *ns );
int nets_sourceid( struct nstream *ns );

int nets_sourceid2( struct unode *un );

int isonline( void );

void setup_useragent( void );

#endif /* VOYAGER_NETWORK_H */
