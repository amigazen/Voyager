/**************************************************************************

  =======================
  The Voyager Web Browser
  =======================

  Copyright (C) 1995-2003 by
   Oliver Wagner <owagner@vapor.com>
   All Rights Reserved

  Parts Copyright (C) by
   David Gerber <zapek@vapor.com>
   Jon Bright <jon@siliconcircus.com>
   Matt Sealey <neko@vapor.com>

**************************************************************************/


//#include <proto/dos.h>
//#include <proto/exec.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

	typedef char *STRPTR;
	typedef unsigned short UWORD;
	typedef signed short WORD;
	typedef unsigned long ULONG;
	typedef long LONG;
	typedef void *APTR;

	struct ListNode
	{
		struct  ListNode *ln_Succ;          // Pointer to next ListNode (Successor)
		struct  ListNode *ln_Pred;          // Pointer to previous ListNode (Predecessor)
		STRPTR  ln_Name;                    // Pointer to a C String
		UWORD   ln_Type;                    // A type number
		WORD    ln_Priority;                // Signed priority, for sorting
	};
	typedef struct ListNode ListNode_s;     // A handy short cut
	typedef struct ListNode *ListNode_p;    // A handy short cut

	struct TinyNode
	{
		struct TinyNode *tn_Succ;               // Pointer to next TinyNode (Successor)
		struct TinyNode *tn_Pred;               // Pointer to previous TinyNode (Predecessor)
	};
	typedef struct TinyNode TinyNode_s;         // A handy short cut
	typedef struct TinyNode *TinyNode_p;        // A handy short cut
#define MinNode TinyNode

	struct ListHead
	{
		struct  ListNode *lh_Head;              // /->Next
		struct  ListNode *lh_Tail;              // |  Prev  Next (=PrevNext)= NULL
		struct  ListNode *lh_TailPred;      // \------->Prev
	};

	struct MinList
	{
		struct  TinyNode *lh_Head;              // /->Next
		struct  TinyNode *lh_Tail;              // |  Prev  Next (=PrevNext)= NULL
		struct  TinyNode *lh_TailPred;      // \------->Prev
	};

	#define IsListEmpty(x)  ( IsNodeTail((x)->lh_Head) )

	#define IsNodeTail(x)   ((((struct MinNode *)(x))->tn_Succ) == NULL )
	#define IsNodeHead(x)   ((((struct MinNode *)(x))->tn_Pred) == NULL )
	#define IsNodeReal(x)   (!(IsNodeTail(x) || IsNodeHead(x) ))
	#define GetNextNode(x)  (((struct MinNode *)(x))->tn_Succ )
	#define GetPrevNode(x)  (((struct MinNode *)(x))->tn_Pred )

void
NewList(struct MinList *list)
{
	list->lh_Head       = (struct MinList*)&(list->lh_Tail);
	list->lh_Tail               = NULL;
	list->lh_TailPred   = (struct MinList*)list;
}

void
AddNode(struct MinList *list,struct MinNode *node,struct MinNode *listNode)
{
struct MinNode *next;

	if ( !listNode )
		listNode = (struct MinNode*)list;

		if ( IsNodeTail(listNode))              // Is listNode the end of list ?
				listNode = listNode->tn_Pred;   // Move listNode one node back.

	next = GetNextNode(listNode);

	//*********** Now Put our node between listNode and next Node

	node->tn_Succ       = next;
	node->tn_Pred       = listNode;

	listNode->tn_Succ   = node;
	next->tn_Pred       = node;
}

void
AddHead(struct MinList *list, struct MinNode *node)
{
struct MinNode *first;
	first           =   list->lh_Head;      // get first node
	first->tn_Pred  =   node;               // link first node to our node
	node->tn_Succ   =   first;              // link our node
	node->tn_Pred   =   (struct MinNode *)list;
	list->lh_Head   =   node;               // point to new first node
}

void
AddTail(struct MinList *list, struct MinNode *node)
{
struct MinNode *last;
	last            =   list->lh_TailPred;              // Get last node
	last->tn_Succ   =   node;                           // Link our node at end of list
	node->tn_Pred   =   last;                           // Setup node
	node->tn_Succ   =   (struct MinNode *)&list->lh_Tail;     // Points to our dummy tail node
	list->lh_TailPred   =   node;                       // point to new last node
}

#define ISLISTEMPTY IsListEmpty
#define NEWLIST(x) NewList(x)
#define ADDTAIL(l,n) AddTail((struct MinList*)l,(struct MinNode*)n)
#define ADDHEAD(l,n) AddHead((struct MinList*)l,(struct MinNode*)n)
#define FIRSTNODE(l) ((APTR)((struct MinList*)l)->lh_Head)
#define NEXTNODE(n) ((APTR)((struct MinNode*)n)->tn_Succ)
#define RETURN_ERROR -1
#define Insert(a,b,c) AddNode(a,b,c)

struct stringnode {
	struct MinNode n;
	struct MinNode on;
	char label[ 80 ];
	int len;
	struct stringnode *part;
	int partoffset;
	int number;
	char text[ 0 ];
};

static char buffer[ 8192 ], buffer2[ 256 ], buffer3[ 512 ];

static struct MinList sl, osl;

static int maxlen;

static void readstrings( FILE *inf )
{
	char *p;
	struct stringnode *n, *rn;
	int linecnt = 0;
	int number = 0;
	int size = 0;

	for(;;)
	{
		if( !fgets( buffer2, sizeof( buffer2 ), inf ) )
		{
			break;
		}
		linecnt++;
		if( buffer2[ 0 ] == ';' )
			continue;

		p = strchr( buffer2, ' ' );
		if( !p )
		{
			printf( "error in line %ld: %s", linecnt, buffer2 );
			exit( RETURN_ERROR );
		}
		else
		{
			*p = 0;
		}

		buffer[ 0 ] = 0;

		// now read actual string data
		for(;;)
		{
			if( !fgets( buffer3, sizeof( buffer3 ), inf ) )
				break;
			linecnt++;
			p = strrchr( buffer3, '\n' );
			if( p )
				*p = 0;
			if( buffer3[ 0 ] )
			{
				p = strchr( buffer3, 0 ) - 1;
				if( *p == '\\' )
				{
					*p = 0;
					strcat( buffer, buffer3 );
				}
				else
				{
					strcat( buffer, buffer3 );
					break;
				}
			}
		}

		// buffer has string, buffer2 has label
		p = buffer;
		while( *p )
		{
			if( *p == '\\' ) switch( p[ 1 ] )
			{
				case 'n':
					*p = '\n';
					strcpy( p + 1, p + 2 );
					break;

				case 'r':
					*p = '\r';
					strcpy( p + 1, p + 2 );
					break;

				case '\\':
					*p = '\\';
					strcpy( p + 1, p + 2 );
					break;

				case '"':
					*p = '"';
					strcpy( p + 1, p + 2 );
					break;

				case 'x':
					{
						char bf[ 4 ];
						LONG v;

						bf[ 0 ] = p[ 2 ];
						bf[ 1 ] = p[ 3 ];
						bf[ 2 ] = 0;

						v=strtol( bf, NULL, 16 );
						if( !v )
							v = 0xff;
						*p = v;
						strcpy( p + 1, p + 4 );
					}
					break;

				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
					{
						char bf[ 4 ];
						LONG v;

						bf[ 0 ] = p[ 1 ];
						bf[ 1 ] = p[ 2 ];
						bf[ 2 ] = p[ 3 ];
						bf[ 3 ] = 0;

						v=strtol( bf,NULL,8 );
						if( !v )
							v = 0xff;
						*p = v;
						strcpy( p + 1, p + 4 );
					}
					break;

				default:
					printf( "unknown format code in line %ld: %s\n", linecnt, p );
					exit( 20 );
				
			}
			p++;
		}

		n = malloc( sizeof( *n ) + strlen( buffer ) + 1 );
		strcpy( n->label, buffer2 );
		strcpy( n->text, buffer );
		n->len = strlen( buffer );
		size += n->len;

		n->part = NULL;
		n->partoffset = 0;

		n->number = number++;

		ADDTAIL( &osl, &n->on );

		// add node
		if( ISLISTEMPTY( &sl ) )
			ADDTAIL( &sl, n );
		else
		{
			struct stringnode *rn2 = FIRSTNODE( &sl );

			if( n->len > rn2->len )
				ADDHEAD( &sl, n );
			else
			{
				for( rn = rn2, rn2 = NEXTNODE( rn2 ); NEXTNODE( rn2 ); rn = rn2, rn2 = NEXTNODE( rn2 ) )
				{
					if( n->len >= rn2->len )
					{
						Insert( (struct List*)&sl, (struct Node*)n, (struct Node*)rn );
						break;
					}
				}
				if( !NEXTNODE( rn2 ) )
					ADDTAIL( &sl, n );
			}
		}
	}

	printf( "read %ld strings, %ld bytes\n", number, size );
}

static void optimizestrings( void )
{
	struct stringnode *n;
	int optcnt = 0, optsize = 0;

	for( n = FIRSTNODE( &sl ); NEXTNODE( n ); n = NEXTNODE( n ) )
	{
		struct stringnode *cmp;

		for( cmp = FIRSTNODE( &sl ); cmp != n; cmp = NEXTNODE( cmp ) )
		{
			if( !cmp->part && !strncmp( &cmp->text[ cmp->len - n->len ], n->text, n->len ) )
			{
				// found
				//printf( "found match '%s' (%s) in '%s' (%s)\n", n->text, n->label, cmp->text, cmp->label );
				n->part = cmp;
				n->partoffset = cmp->len - n->len;
				optcnt++;
				optsize += n->len;
				break;
			}
		}
	}
	printf( "optimized %ld strings, %ld bytes\n", optcnt, optsize );
}

static void writehfile( FILE *hf )
{
	struct stringnode *n;
	int cnt = 0;

	for( n = FIRSTNODE( &sl ); NEXTNODE( n ); n = NEXTNODE( n ), cnt++ )
		fprintf( hf, "#define %s %ld\n", n->label, n->number );

	fprintf( hf, "#define NUMCATSTRING %ld\n", cnt );
}

static void writeasmfile( FILE *af )
{
	struct stringnode *n;

	fprintf( af, "\tsection _NOMERGE,data\n" );

	for( n = FIRSTNODE( &sl ); NEXTNODE( n ); n = NEXTNODE( n ) )
	{
		if( !n->part )
		{
			char *p = n->text;
			int cnt = 0;
			fprintf( af, "msg%ld: ", n->number );
			while( *p )
			{
				if( !cnt )
					fprintf( af, " dc.b " );
				else
					fprintf( af, "," );
				if( *p == 0xff )
					fprintf( af, "0", p++ );
				else
					fprintf( af, "%lu", *p++ );
				if( cnt++ == 20 )
				{
					fprintf( af, "\n" );
					cnt = 0;
				}
			}
			fprintf( af, "\n\tdc.b 0\n" );
		}
	}

	fprintf( af, "\tsection __MERGED,data\n\txdef ___stringtable\n___stringtable:\n" );

	for( n = FIRSTNODE( &osl ); NEXTNODE( n ); n = NEXTNODE( n ) )
	{
		n = (APTR)((ULONG)n-8);
		if( !n->part )
			fprintf( af, "\tdc.l msg%ld\n", n->number );
		else
			fprintf( af, "\tdc.l msg%ld+%ld\n", n->part->number, n->partoffset );
		n = (APTR)((ULONG)n+8);
	}

	fprintf( af, "\n\tend\n" );
}

static void writecfile( FILE *cf )
{
	struct stringnode *n;
	int    stroffset=0;
	int    cnt=0;

	fprintf( cf, "char __strings[]=\n\t\"");

	for( n = FIRSTNODE( &sl ); NEXTNODE( n ); n = NEXTNODE( n ) )
	{
		n->number=stroffset;

		if( !n->part )
		{
			char *p = n->text;
			while( *p )
			{
				fprintf( cf, "\\x%02x", *p==0xff ? 0 : *p);

				p++;
				stroffset++;

				if (cnt++ == 20)
				{
					fprintf( cf, "\"\\\n\t\"");
					cnt=0;
				}
			}
			fprintf( cf, "\\x00" );
			stroffset++;
		}
	}

	fprintf( cf, "\";\n\n");

	fprintf( cf, "char *__stringtable[] = {\n\t");

	for( n = FIRSTNODE( &osl ); NEXTNODE( n ); n = NEXTNODE( n ) )
	{
		n = (APTR)((ULONG)n-8);
		if( !n->part )
			fprintf( cf, "__strings+%ld", n->number );
		else
			fprintf( cf, "__strings+%ld", n->part->number+n->partoffset );


		n = (APTR)((ULONG)n+8);

		if( NEXTNODE( NEXTNODE( n ) ) )
			fprintf( cf, ",\n\t" );
	}
	fprintf( cf, "\n};\n\n" );
}

void main( int argc, char **argv )
{
	FILE *inf, *hf, *af, *cf;

	NEWLIST( &sl );
	NEWLIST( &osl );

	if( argc < 3 || argc > 4 )
	{
		printf( "usage: catmaker cdfile .h-file ['cfile']\n" );
		exit( 20 );
	}

	inf = fopen( argv[ 1 ], "r" );
	if( !inf )
	{
		printf( "can't open %s\n", argv[ 1 ] );
		exit( 10 );
	}

	hf = fopen( argv[ 2 ], "w" );
	if( !hf )
	{
		printf( "can't open %s\n", argv[ 2 ] );
		exit( 10 );
	}

	if (argc==3)
	{
		af = fopen( "/tmp/cattmp.a", "w" );
		if( !af )
		{
			printf( "can't open T:cattmp.a\n" );
			exit( 10 );
		}
	}
	else
	{
		cf = fopen( "cattmp.c", "w" );
		if( !cf )
		{
			printf( "can't open T:cattmp.c\n" );
			exit( 10 );
		}
	}

	printf( "reading strings...\n" );
	readstrings( inf );
	printf( "optimizing strings...\n" );
	optimizestrings();
	printf( "writing .h file...\n" );
	writehfile( hf );

	if (argc==3)
	{
		printf( "writing .a file...\n" );
		writeasmfile( af );
	}
	else
	{
		printf( "Writing .c file...\n" );
		writecfile( cf);
	}

	printf( "done\n" );
	exit( 0 );
}
