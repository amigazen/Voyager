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


//
// $Id: fontcache.c,v 1.27 2003/07/06 16:51:33 olli Exp $
//

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/exec.h>
#include <proto/diskfont.h>
#include <proto/graphics.h>
#endif

/* private */
#include "malloc.h"
#include "prefs.h"
#include "mui_func.h"


struct fontnode {
	struct MinNode n;
	char name[ 32 ];
	struct TextFont *tf;
#ifndef MBX
	UBYTE fontarray[ 257 ];
#endif
};

struct fontcachenode {
	struct MinNode n;
	struct TextFont *tf;
	UBYTE *fontarray;
	int sizespec;
	int namelen;
	char name[ 0 ];
};

static struct MinList fontlist, fontcachelist;

void init_fontlist( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	NEWLIST( &fontlist );
	NEWLIST( &fontcachelist );
}

void cleanup_fonts( void )
{
	struct fontnode *fn;

	D( db_init, bug( "cleaning up..\n" ) );

	while( fn = REMHEAD( &fontlist ) )
	{
		CloseFont( fn->tf );
	}
}

void cleanup_fontcache( void )
{
	struct fontcachenode *fcn;
	
	while( fcn = REMHEAD( &fontcachelist ) )
		free( fcn );
}

struct TextFont *myopenfont( char *name, char **fontarray )
{
	struct fontnode *fn;
	struct TextAttr ta = { 0 };
	char buffer[ 128 ];
	char *p;
	extern void makefontarray( struct TextFont *tf, UBYTE *fa );

	for( fn = FIRSTNODE( &fontlist ); NEXTNODE( fn ); fn = NEXTNODE( fn ) )
	{
		if( !strcmp( fn->name, name ) )
		{
#ifndef MBX
			*fontarray = fn->fontarray;
#endif
			return( fn->tf );
		}
	}

	stccpy( buffer, name, sizeof( buffer ) );
	p = strrchr( buffer, '/' );
	if( p )
	{
		*p++ = 0;
		ta.ta_YSize = atoi( p );
	}
	else
		ta.ta_YSize = 8;

	ta.ta_Name = buffer;

#ifdef MBX
	ta.ta_Flags |= FPF_ALPHA_BGPEN;
#endif

	if( strlen( buffer ) < 5 || stricmp( &buffer[ strlen( buffer ) - 5 ], ".font" ) )
		strcat( buffer, ".font" );

	fn = malloc( sizeof( *fn ) );
	memset( fn, '\0', sizeof( *fn ) );

    ADDTAIL( &fontlist, fn );
	stccpy( fn->name, name, sizeof( fn->name ) );

#if USE_DOS
	fn->tf = OpenDiskFont( &ta );
	if( !fn->tf )
#endif
	{
		fn->tf = OpenFont( &ta );
		if( !fn->tf )
		{
#ifdef MBX
			ta.ta_Name = "romprop.font";
			ta.ta_YSize = 13;
#else
			ta.ta_Name = "topaz.font";
			ta.ta_YSize = 8;
#endif
			fn->tf = OpenFont( &ta );
		}
	}

#ifndef MBX
	makefontarray( fn->tf, fn->fontarray );
	*fontarray = fn->fontarray;
#endif

	return( fn->tf );
}

static struct TextFont *setupfont( int st_font, char *st_face, UBYTE **cfa, int usetemplate )
{
	int cf;
	int nf = getprefslong( DSI_FONT_FACE_NUM, 3 );
	char tmpname[ 128 ], *p;
	int tmpsize;
	struct TextFont *tf;

	if( st_font < 0 )
		st_font = 0;
	else if( st_font > 6 )
		st_font = 6;

	//Printf( "setupfont(%ld,%s)\n", st_font, st_face );

	for( cf = 0; cf < nf; cf++ )
	{
		if( !stricmp( getprefsstr( DSI_FONT_FACENAME( cf ), "" ), st_face ) )
		{
			char *afname  = getprefsstr( DSI_FONT_MAP( cf, st_font ), "" );
			//Printf( "found direct mapping %s\n", afname );
			return( myopenfont( afname, (char**)cfa ) );
		}
	}

	if( !usetemplate )
		return( NULL );

	// No direct mapping
	// Use template
	strcpy( tmpname, getprefsstr( DSI_FONT_MAP( 2, st_font ), "topaz/8" ) );
	p = strchr( tmpname, '/' );
	if( !p )
		return( 0 );
	tmpsize = atoi( ++p );
	sprintf( tmpname, "%s/%d", st_face, tmpsize );
	while( p = strpbrk( tmpname, " " ) )
		strcpy( p, p + 1 );

	//Printf( "using template mapping %s\n", tmpname );

	tf = myopenfont( tmpname, (char**) cfa );
	if( tf )
	{
		p = strchr( tmpname, '/' );
		*p = 0;
		if( strnicmp( tf->tf_Message.mn_Node.ln_Name, tmpname, strlen( tmpname ) ) )
			return( 0 );
	}
	return( tf );
}

struct TextFont *getfont( char *facespec, int sizespec, UBYTE **cfa )
{
	struct TextFont *currentfont = NULL;
	char *p;
	struct fontcachenode *fcn;
	int facespeclen = strlen( facespec );

	for( fcn = FIRSTNODE( &fontcachelist ); NEXTNODE( fcn ); fcn = NEXTNODE( fcn ) )
	{
		if( fcn->namelen == facespeclen && fcn->sizespec == sizespec && !strcmp( fcn->name, facespec ) )
		{
			*cfa = fcn->fontarray;
			return( fcn->tf );
		}
	}

	fcn = malloc( sizeof( *fcn ) + facespeclen + 1 );
	memset( fcn, '\0', sizeof( *fcn ) + facespeclen + 1 ); /* TOFIX: maybe not necessary */
    strcpy( fcn->name, facespec );
	fcn->sizespec = sizespec;
	fcn->namelen = facespeclen;

	D( db_html, bug( "getfont(%s,%ld)\n", facespec, sizespec ));

	for( p = strtok( facespec, "," ); p; p = strtok( NULL, "," ) )
	{
		char *p2;

		p = stpblk( p );
		p2 = strchr( p, 0 ) - 1;
		while( p2 > p && isspace( *p2 ) ) 
			*p2-- = 0;

		currentfont = setupfont( sizespec, p, cfa, FALSE );
		D( db_html, bug( "trying(%s,%ld), got %lx\n", p, sizespec, currentfont ) );
		if( currentfont )
			break;

		currentfont = setupfont( sizespec, p, cfa, TRUE );
		D( db_html, bug( "trying(%s,%ld) as template, got %lx\n", p, sizespec, currentfont ) );
		if( currentfont )
			break;
	}
	if( !currentfont )
	{
		currentfont = setupfont( sizespec, "(Default)", cfa, TRUE );
	}

	fcn->tf = currentfont;
	fcn->fontarray = *cfa;

	ADDTAIL( &fontcachelist, fcn );

	D( db_html, bug( "-> selected %s/%ld\n", currentfont->tf_Message.mn_Node.ln_Name, currentfont->tf_YSize ) );

	return( currentfont );
}
