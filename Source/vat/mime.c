/*
 * $Id: mime.c,v 1.20 2001/11/13 22:20:02 owagner Exp $
 */

#define __USE_SYSBASE
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <proto/intuition.h>
#include <proto/rexxsyslib.h>
#include <proto/muimaster.h>
#include <proto/asl.h>
#include <proto/datatypes.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <libraries/mui.h>
#include <devices/timer.h>
#include <string.h>
#include <dos/dostags.h>
#include <macros/vapor.h>
#include <string.h>
#include <stdlib.h>

#include "globals.h"

#define NO_VAT_SHORTCUTS
#include "libraries/vat.h"

static struct MinList l;
static APTR mimepool;
static struct SignalSemaphore mimesem;

struct mimeextension {
	struct MinNode n;
	STRPTR ext;
};

struct mimeentry {
	struct MinNode n;
	ULONG action;          /* what to do */
	STRPTR type;           /* mime type */
	struct MinList extl;   /* extension list */
	STRPTR app;            /* app for viewing */
	STRPTR path;           /* download path */
};

/*
 * Reloads the prefs from disk.
 */
void loadmimeprefs( int lock ) /* TOFIX: return something.. */
{
	BPTR f;
	STRPTR p, q;

	DB( ( "called!\n" ) );

	if( lock )
	{
		ObtainSemaphore( &mimesem );
	}

	if( mimepool )
	{
		DeletePool( mimepool );
		mimepool = 0;
	}

	NEWLIST( &l );

	f = Open( "ENV:MIME.prefs", MODE_OLDFILE );
	if( !f )
	{
		if( lock )
		{
			ReleaseSemaphore( &mimesem );
		}
		return; /* TOFIX: etc.. */
	}

	DB( ( "loading mimeprefs..\n" ) );

	mimepool = CreatePool( MEMF_ANY, 2048, 1024 );

	if( f )
	{

		if( mimepool )
		{
			char buffer[ 512 ]; /* tsk tsk.. */
			STRPTR last_class_dir = NULL;
			STRPTR download_path;

			while( FGets( f, buffer, sizeof( buffer ) - 1 ) )
			{
				struct mimeentry *me;
				int is_class = FALSE;

				/* skip comments */
				if( buffer[ 0 ] == ';' )
				{
					DB( ( "skiping comments\n" ) );
					continue;
				}

				q = stpbrk( buffer, "\r\n" );
				if( q )
				{
					*q = 0;
				}
				
				DB( ( "processing: %s\n", buffer ) );

				me = ( struct mimeentry * )AllocPooled( mimepool, sizeof( *me ) );
				if( me )
				{
					int x = FALSE;
					/*
					 * Fill-in the fields.
					 */
					DB( ( "filling in the fields\n" ) );

					/* mimetype */
					if( !( q = strchr( buffer, ',' ) ) )
					{
						FreePooled( mimepool, me, sizeof( *me ) );
						continue; /* TOFIX: this is wrong. and are we allowed to put an entry without mimetype ? */
					}
					*q++ = '\0';

					p = buffer;

					if( ( q - p > 1 ) && ( me->type = AllocPooled( mimepool, strlen( buffer ) + 1 ) ) )
					{
						strcpy( me->type, buffer );

						if( strchr( me->type, '*' ) )
						{
							is_class = TRUE;
						}

						DB( ( "mimetype: %s\n", me->type ) );

						p = q;

						NEWLIST( &me->extl );

						/* extension */
						if( ( q = strchr( p, ',' ) ) )
						{
							x = TRUE;
							*q++ = '\0';
							if( q - p > 1 )
							{
								struct mimeextension *mext;
								STRPTR s = p;
								
								x = FALSE;

								DB( ( "found extentions %s\n", s ) );

								/* ok, there are some */
								while ( !x )
								{
									if( *p == ' ' )
									{
										*p = '\0';
									}
									else if( *p == '\0' )
									{
										x = TRUE;
									}
									else
									{
										p++;
										continue;
									}

									if( ( mext = ( struct mimeextension * )AllocPooled( mimepool, sizeof( struct mimeextension ) ) ) && ( mext->ext = ( STRPTR )AllocPooled( mimepool, p - s + 1 ) ) )
									{
										DB( ( "adding %s to list\n", s ) );
										strcpy( mext->ext, s );
										ADDTAIL( &me->extl, mext );
										s = p + 1;
									}
									else
									{
										/* argh, out of mem */
										x = FALSE;
										break;
									}
									p++;
								}
							}
							
							if( x )
							{
								p = q;
								
								/* download path */
								if( ( q = strchr( p, ',' ) ) )
								{
									*q++ = '\0';
									if( q - p > 1 )
									{
										DB( ( "found download path: %s\n", p ) );
										download_path = p;
									}
									else
									{
										download_path = 0;
									}
									
									p = q;

									/* viewer */
									if( ( q = strchr( p, ',' ) ) )
									{
										DB( ( "found viewer placeholder\n" ) );
										*q++ = '\0';
										if( q - p > 1 )
										{
											if( ( me->app = AllocPooled( mimepool, strlen( p ) + 1 ) ) )
											{
												DB( ( "viewapp: %s\n", p ) );
												strcpy( me->app, p );
											}
											else
											{
												DB( ( "allocation failure for viewapp\n" ) );
												x = FALSE;
											}
										}
										else
										{
											me->app = NULL;
										}

										if( x )
										{
											p = q;

											DB( ( "viewmode ?\n" ) );

											/* viewmode */
											if( ( q = strchr( p, ',' ) ) )
											{
												DB( ( "found viewmode placeholder\n" ) );
												*q++ = '\0';
												if( q - p > 1 )
												{
													me->action = atoi( p );

													p = q;
														
													DB( ( "classdir ?\n" ) );

													/* use_classdir */
													if( ( q = strchr( p, ',' ) ) )
													{
														DB( ( "found classdir placeholder\n" ) );
														*q++ = '\0';
														if( q - p > 1 )
														{
															DB( ( "ok..\n" ) );
															if( *p == '1' && !is_class )
															{
																DB( ( "using class dir: %s\n", last_class_dir ) );
																me->path = last_class_dir;
															}
															else
															{
																if( download_path )
																{
																	if( ( me->path = ( STRPTR )AllocPooled( mimepool, strlen( download_path ) + 1 ) ) )
																	{
																		strcpy( me->path, download_path );
																	}
																	else
																	{
																		x = FALSE;
																	}
																}
																else
																{
																	me->path = NULL;
																}
															}
															
															DB( ( "still here..\n" ) );

															if( x )
															{
																/* update class specification */
																if( is_class )
																{
																	last_class_dir = me->path;
																}
																
																p = q;

																DB( ( "pipe stream ?\n" ) );

																/* pipe stream */
																if( ( q = strchr( p, ',' ) ) )
																{
																	DB( ( "pipe placeholder found\n" ) );
																	*q++ = '\0';
																	if( q - p > 1 )
																	{
																		DB( ( "still ok.. close\n" ) );
																		if( *p == '1' )
																		{
																			me->action |= MF_PIPE_STREAM;
																		}

																		/* view inline (ok, this is the last flag) */
																		DB( ( "last flag : %s\n", q ) );
																		if( *q == '1' )
																		{
																			DB( ( "found view inline!\n" ) );
																			me->action |= MF_VIEW_INLINE;
																		}

																		DB( ( "adding entry !\n" ) );
																		ADDTAIL( &l, me );
																	}
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
					if( !x )
					{
						//displaybeep(); /* TOFIX: add a requester or something */
						DeletePool( mimepool );
						mimepool = NULL;
					}
				}
			}
		}
		Close( f );
	}

	if( lock )
	{
		DB( ( "releasing semaphore\n" ) );
		ReleaseSemaphore( &mimesem );
	}
	DB( ( "returning..\n" ) );
}

void initmimeprefs( void )
{
	InitSemaphore( &mimesem );
}

void exitmimeprefs( void )
{
	if( mimepool )
	{
		DeletePool( mimepool );
	}
}

struct mimetab {
	STRPTR ext;
	STRPTR mime;
};

/*
 * Built-in rules.
 */
static struct mimetab mt[] = {
	"htm", "text/html",
	"html", "text/html",
	"txt",  "text/plain",
	"gif",  "image/gif",
	"jpg",  "image/jpeg",
	"jpeg", "image/jpeg",
	"jfif", "image/jpeg",
	"png",  "image/png",
	"xbm",  "image/xbm",
	NULL, NULL
};

/*
 * Then we need a built-in mimeentry
 * too.
 */
static struct mimeentry gme;

/*
 * Returns the mimetype if found.
 * Pass either mimetype or extension (mutually
 * exclusive).
 */
struct mimeentry * find_mimeentry( STRPTR mimetype, STRPTR extension )
{
	struct mimeentry *me;
	STRPTR ext;

	DB( ( "called\n" ) );

	if( extension )
	{
		ext = strrchr( extension, '.' );
		if( !ext )
			return( FALSE );
		ext++;
		DB( ( "checking for extension %s\n", ext ) );
	}

	if( !mimepool )
	{
		DB( ( "no mimepool, calling loadmimeprefs..\n" ) );
		loadmimeprefs( FALSE );
		DB( ( "returned ok\n" ) );
	}

	DB( ( "ok, hm..\n" ) );

	if( !mimepool && extension )
	{
		/*
		 * Oh well, something went wrong. Using
		 * built-in rules. We only do that for extension searching.
		 */
		int c;

		DB( ( "something went wrong, using built in rules\n" ) );

		for( c = 0; mt[ c ].ext; c++ )
		{
			if( !strnicmp( mt[ c ].ext, ext, strlen( mt[ c ].ext ) ) )
			{
				gme.action = MT_ACTION_VIEW | MF_VIEW_INLINE;
				gme.type = mt[ c ].mime;

				return( &gme );
			}
		}
		return( 0 );
	}

	DB( ( "iterating the list..\n" ) );

	ITERATELIST( me, &l )
	{

		if( extension )
		{
			struct mimeextension *mext;

			ITERATELIST( mext, &me->extl )
			{
				DB( ( "iterating: %s\n", mext->ext ) );
				if( !stricmp( mext->ext, ext ) )
				{
					DB( ( "found extention %s!\n", mext->ext ) );
					return( me );
				}
			}
		}
		else
		{
			if( !stricmp( me->type, mimetype ) )
			{
				DB( ( "found!\n" ) );
				DB( ( "structure action: %ld\n", me->action ) );
				return( me );
			}
		}
	}
	DB( ( "returning..\n" ) );
	return( NULL );
}


/*
 * Automatically fills in the mime fields.
 * Mainly useful for the braindead VAT_MIME_Find*
 * functions. Supply filename if you want to search
 * by extention, otherwise it means it's by
 * mimetype.
 */
int fill_mimeentry(
	char *filename,
	char *savedir,
	char *viewer,
	int *viewmode,
	char *mimetype
)
{
	struct mimeentry *me;
	int found = FALSE;
	
	ObtainSemaphoreShared( &mimesem );

	DB( ( "called\n" ) );


	if( filename )
	{
		DB( ( "using extension mode\n" ) );
		if( ( me = find_mimeentry( NULL, filename ) ) )
		{
			found = TRUE;
		}
		else
		{
			/*
			 * Extension was empty.. Happens with http://www.blah.com/ and ignore
			 * server-sent mimetype. So will NULL terminate mimetype.
			 */
			if( mimetype )
			{
				mimetype[ 0 ] = '\0';
			}
		}
	}
	else
	{
		char tmpmimetype[ 256 ], *p;

		stccpy( tmpmimetype, mimetype, sizeof( tmpmimetype ) );
		p = strchr( tmpmimetype, ';' );
		if( p )
			*p = 0;

		DB( ( "using mimetype mode and giving mimetype %s\n", mimetype ) );
		if( ( me = find_mimeentry( tmpmimetype, NULL ) ) )
		{
			found = TRUE;
		}
	}
	
	DB( ( "ok, found == %ld\n", found ) );

	DB( ( "filename: %s\n", filename ) );

	if( found )
	{
		if( filename && mimetype )
		{
			if( me->type )
			{
				stccpy( mimetype, me->type, 256 );
			}
			else
			{
				*mimetype = '\0';
			}
		}
		
		if( viewmode )
		{
			*viewmode = me->action;
		}

		if( savedir )
		{
			if( me->path )
			{
				DB( ( "path it %s\n", me->path ) );
				stccpy( savedir, me->path, 256 );
			}
			else
			{
				*savedir = '\0';
			}
		}
	
		if( viewer )
		{
			if( me->app )
			{
				stccpy( viewer, me->app, 256 );
			}
			else
			{
				*viewer = '\0';
			}
		}
		DB( ( "filename: %s\n", filename ) );
	}

	/* TOFIX: nullify the other fields.. check if it's needed and document that shit */

	DB( ( "about to return..\n" ) );
	
	ReleaseSemaphore( &mimesem );

	return( found );
}

int ASM SAVEDS VAT_MIME_FindByExtension(
	__reg( a0, char *filename ),
	__reg( a1, char *savedir ),
	__reg( a2, char *viewer ),
	__reg( a3, int *viewmode ),
	__reg( d0, char *mimetype )
)
{
	DB( ( "ok, filename: %s\n", filename ) );
	return( fill_mimeentry( filename, savedir, viewer, viewmode, mimetype ) );
}

int ASM SAVEDS VAT_MIME_FindByType(
	__reg( a0, char *mimetype ),
	__reg( a1, char *savedir ),
	__reg( a2, char *viewer ),
	__reg( a3, int *viewmode )
)
{
	DB( ( "called with mimetype == %s\n", mimetype ) );
	return( fill_mimeentry( NULL, savedir, viewer, viewmode, mimetype ) );
}
