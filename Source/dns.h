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


#ifndef VOYAGER_DNS_H
#define VOYAGER_DNS_H
/*
 * $Id: dns.h,v 1.6 2001/07/01 22:02:38 owagner Exp $
 */

#define DNSTASKS 4 /* number of concurrent DNS processes */

struct dnscachenode {
	struct dnscachenode *next;
	ULONG *ip;
	UWORD namelen;
	UBYTE http11failed;
	char name[ 0 ];
};

struct dnsmsg {
	struct Message m;
	char *name;
	struct dnscachenode *dcn; // created DNS cache node
};

/* protos */
extern struct MsgPort *dnsport[ DNSTASKS ];
extern struct Process *dnsproc[ DNSTASKS ];
extern struct MsgPort *dnsreply;
extern struct dnscachenode *dnscachelist;
extern UWORD dnshandler_die, nethandler_die;
extern struct SignalSemaphore dnscachesem;
void SAVEDS dnshandler( void );
struct dnscachenode *dnscache_find( char *name );
void dnsmsg_queue( struct dnsmsg *dm );

#endif /* VOYAGER_DNS_H */
