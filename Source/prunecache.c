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


/*
 * Cache prunning class
 * --------------------
 * - Allows to launch a cache prunning process which will report
 *   its progress to a GUI (or not if automatically launched).
 *   It's possible to select how much will be cleaned up, etc...
 *
 * © 2000 by VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: prunecache.c,v 1.30 2003/11/21 07:48:50 zapek Exp $
 *
*/

#include "voyager.h"

#if USE_NET

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <exec/memory.h>
#include <proto/exec.h>
#endif

/* private */
#include "cache.h"
#include "classes.h"
#include "voyager_cat.h"
#include "errorwin.h"
#include "splashwin.h"
#include "methodstack.h"
#include "time_func.h"
#include "prefs.h"
#include "mui_func.h"
#include "dos_func.h"


/* global data */
APTR prunecachewin;
struct SignalSemaphore prunecachesem;
ULONG prunecacheinit;
extern int cache_last_prune;
static LONG prune_max_size = -1;
struct cpnode {
	int size;
	time_t age;
	int cdir;
	UBYTE name[ 32 ];
};

/* instance data */
struct Data {
	APTR bt_action;
	APTR ga_fill;
	APTR sl_value;
	APTR txt_status;
	LONG cache_size;
};

struct Process *prunecacheproc;

/*
 * qsort() to compare dates
 */
#if USE_DOS
static int cpcmpfunc( const void *p1, const void *p2 )
{
	struct cpnode *cp1 = *((struct cpnode**)p1);
	struct cpnode *cp2 = *((struct cpnode**)p2);

	return( cp1->age - cp2->age );
}
#endif

#define EAC_BUFFERSIZE 2048 /* ExAll buffer */

/*
 * Clean up the cache to reduce it to maxsize.
 */
void prunecache( void )
{
#if USE_DOS
	struct MinList l;
	APTR cpool;
	int c;
	struct Process *me = (struct Process*)FindTask( NULL );
	int totalsize = 0;
	int totalcount = 0;
	struct cpnode **cpa;
	int cpasize = 1024;
	int brk = FALSE;
	BPTR oldlock = me->pr_CurrentDir;
	struct ExAllControl *eac;
	struct ExAllData *ead;
	APTR ex_buffer;
#if USE_EXECUTIVE
	APTR msg;
#endif /* USE_EXECUTIVE */

	NEWLIST( &l );
	cpool = CreatePool( MEMF_ANY, sizeof( struct cpnode ) * 128, sizeof( struct cpnode ) * 64 );
	if( !cpool )
		return;

	cpa = AllocPooled( cpool, cpasize * 4 );
	if( !cpa )
	{
		DeletePool( cpool );
		return;
	}

	if( ( eac = AllocDosObject( DOS_EXALLCONTROL, NULL ) ) )
	{
		if( ( ex_buffer = malloc( EAC_BUFFERSIZE ) ) )
		{
			int more;

#if USE_EXECUTIVE
			/*
			 * Nice value of +20 for Executive people.
			 */
			msg = InitExecutive();
			if ( msg )
			{
				SetNice( msg, 20 );
				ExitExecutive( msg );
			}
#endif /* USE_EXECUTIVE */

			/*
			 * Scanning cache directory.
			 */
			if( prunecachewin )
			{
				pushmethod( prunecachewin, 2, MM_PrunecacheWin_SetTxt, GS( PRUNECACHEWIN_SCANNING ) );
			}

			for( c = 0; !brk && c < 256; c++ )
			{
				if( CheckSignal( SIGBREAKF_CTRL_C ) )
				{
					brk = TRUE;
					break;
				}

				chcache( c );
				
				eac->eac_LastKey = 0;
				do
				{
					more = ExAll( me->pr_CurrentDir, ex_buffer, EAC_BUFFERSIZE, ED_DATE, eac );

					if( ( !more ) && ( IoErr() != ERROR_NO_MORE_ENTRIES ) )
					{
						displaybeep();
						break;
					}

					if( eac->eac_Entries > 0 )
					{
						struct cpnode *nn;
						struct DateStamp ds;
						ead = ( struct ExAllData * )ex_buffer;
						
						do
						{
							if( CheckSignal( SIGBREAKF_CTRL_C ) )
							{
								brk = TRUE;
								more = FALSE;
								ExAllEnd( me->pr_CurrentDir, ex_buffer, EAC_BUFFERSIZE, ED_DATE, eac );
								break;
							}

							if( ead->ed_Type >= 0 )
							{
								puterror( LT_CACHE, LL_WARNING, 0, ead->ed_Name, GS( ERROR_BOGUS_CACHE_ENTRY ) );
								continue;
							}
							if( strlen( ead->ed_Name ) != 30 )
							{
								/*
								 * Corrupted file or crap which has nothing
								 * to do here.
								 */
								puterror( LT_CACHE, LL_WARNING, 0, ead->ed_Name, GS( ERROR_BOGUS_CACHE_FILE ) );
								DeleteFile( ead->ed_Name );
								continue;
							}

							nn = AllocPooled( cpool, sizeof( *nn ) );
							if( !nn )
								break;

							nn->cdir = c;
							strcpy( nn->name, ead->ed_Name );
							
							ds.ds_Days = ead->ed_Days;
							ds.ds_Minute = ead->ed_Mins;
							ds.ds_Tick = ead->ed_Ticks;

							nn->age = __datecvt( &ds );
							nn->size = ead->ed_Size;

							totalsize += nn->size;

							if( totalcount == cpasize )
							{
								APTR cpan;
								int oldcpasize = cpasize;

								cpasize += 1024;
								cpan = AllocPooled( cpool, cpasize * 4 );
								if( !cpan )
								{
									brk = TRUE;
									more = FALSE;
									ExAllEnd( me->pr_CurrentDir, ex_buffer, EAC_BUFFERSIZE, ED_DATE, eac );
									break;
								}

								CopyMemQuick( cpa, cpan, oldcpasize * 4 );
								FreePooled( cpool, cpa, oldcpasize * 4 );

								cpa = cpan;
							}
							cpa[ totalcount++ ] = nn;
							ead = ead->ed_Next;
						} while( ead );
					}
				} while( more );
			}

			D( db_cacheprune, bug( "totalsize %ld, totalcount %ld\n", totalsize, totalcount ));

			if( brk )
			{
				/*
				 * totalsize is probably wrong. Mark that.
				 */
				totalsize = 0;
			}

			if( prunecachewin && totalsize != estimated_cache_size )
			{
				pushmethod( prunecachewin, 2, MM_PrunecacheWin_SetRealTotalSize, totalsize / 1024 );
				pushmethod( prunecachewin, 2, MM_PrunecacheWin_SetTxt, GS( PRUNECACHEWIN_WRONGSIZE ) );
				pushmethod( prunecachewin, 2, MM_PrunecacheWin_Busy, FALSE );

				brk = TRUE;
			}
			else
			{
				if( prunecachewin )
				{
					pushmethod( prunecachewin, 2, MM_PrunecacheWin_SetTxt, GS( PRUNECACHEWIN_DELETING ) );
				}
			}

			if( !brk )
			{
				/*
				 * Sort old entries first.
				 */
				qsort( cpa, totalcount, 4, cpcmpfunc );

				/*
				 * Remove all the entries until the cache
				 * is smaller than maxsize
				 */
				for( c = 0; c < totalcount && totalsize > prune_max_size; c++ )
				{
					if( CheckSignal( SIGBREAKF_CTRL_C ) )
					{
						brk = TRUE;
						break;
					}

					D( db_cacheprune, bug( "removing %ld/%s, age %lx\n", cpa[ c ]->cdir, cpa[ c ]->name, cpa[ c ]->age ));
					chcache( cpa[ c ]->cdir );
					DeleteFile( cpa[ c ]->name );
					totalsize -= cpa[ c ]->size;
					if( prunecachewin )
					{
						pushmethod( prunecachewin, 3, MM_PrunecacheWin_SetCurrentSize, totalsize / 1024, FALSE );
					}
				}
				if( prunecachewin )
				{
					pushmethod( prunecachewin, 2, MM_PrunecacheWin_SetTxt, GS( PRUNECACHEWIN_FINISHED ) );
					pushmethod( prunecachewin, 2, MM_PrunecacheWin_Busy, FALSE );
				}
			}

			if( brk && totalsize )
			{
				if( prunecachewin )
				{
					pushmethod( prunecachewin, 2, MM_PrunecacheWin_SetRealTotalSize, totalsize / 1024 );
				}
			}
			else
			{
				if( prunecachewin )
				{
					pushmethod( prunecachewin, 3, MM_PrunecacheWin_SetCurrentSize, totalsize / 1024, TRUE );
				}
			}

			estimated_cache_size = totalsize;
			cache_last_prune = timev();
			writeglobalcache();
			free( ex_buffer );
		}
		else
		{
			displaybeep();
		}
		FreeDosObject( DOS_EXALLCONTROL, eac );
	}
	else
	{
		displaybeep();
	}

	DeletePool( cpool );
	CurrentDir( oldlock );

#endif /* USE_DOS */
}

static void SAVEDS prunecachefunc( void )
{
	ObtainSemaphore( &prunecachesem );
	
	if( prune_max_size == -1 )
	{
		prune_max_size = getprefslong( DSI_CACHE_SIZE, 1024 ) * 512; // <- that sucks, change it one day
	}
	prunecache();
	
	
	ReleaseSemaphore( &prunecachesem );
}


void startpruneprocess( void )
{
	D( db_cacheprune, bug( "in startpruneprocess\n" ) );

	prunecacheproc = CreateNewProcTags(
		NP_Entry, prunecachefunc,
		#ifdef __MORPHOS__
		NP_StackSize, 12 * 1024 * 2,
		NP_CodeType, CODETYPE_PPC,
		#else
		NP_StackSize, 12 * 1024,
		#endif
		NP_Name, "V's Cache Cutter",
		NP_Priority, -20,
		NP_Cli, FALSE,
		TAG_DONE
	);
}

void init_prunecache( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	InitSemaphore( &prunecachesem );
	prunecacheinit = TRUE;
}

void start_prunecache( void )
{
	D( db_init, bug( "initializing..\n" ) );

#if USE_SPLASHWIN
	if( use_splashwin )
	{
		DoMethod( splashwin, MM_SplashWin_Update, GS( SPLASHWIN_PRUNECACHE ) );
	}
#endif /* USE_SPLASHWIN */
	
	D( db_cacheprune, bug( "Estimated size %ld, max %ld\n", estimated_cache_size, getprefslong( DSI_CACHE_SIZE, 1024 ) * 1024 ));
	D( db_cacheprune, bug( "tv = %ld, lastprune = %ld\n", timev(), cache_last_prune ));
	D( db_cacheprune, bug( "prune delay == %ld\n", ( 3600 * 24 * getprefslong( DSI_CACHE_AUTOVERIFY, 30 ) ) ));

	/* needs cleaning ? */
	if( getprefslong( DSI_CACHE_SIZE, 0 ) &&
		timev() - cache_last_prune > ( 3600 * 24 * getprefslong( DSI_CACHE_AUTOVERIFY, 30 ) ) &&
		( ( getprefslong( DSI_CACHE_SIZE, 1024 ) * 1024 ) < estimated_cache_size ) )
	{
		D( db_cacheprune, bug( "starting prune\n" ));
		startpruneprocess();
	}
}



/* custom class */
DECNEW
{
	struct Data *data;
	APTR bt_action, ga_fill, sl_value, txt_status;

	LONG cache_size = getprefslong( DSI_CACHE_SIZE, 1024 );

	obj = DoSuperNew(cl, obj,
		MUIA_Window_ID, MAKE_ID( 'P','R','U','N' ),
		MUIA_Window_Title, GS( PRUNECACHEWIN_WINTITLE ),
		MUIA_Window_ScreenTitle, copyright,
		MUIA_Window_NoMenus, TRUE,
		WindowContents, VGroup,
			Child, ga_fill = GaugeObject,
				GaugeFrame,
				MUIA_Gauge_Horiz, TRUE,
				MUIA_Gauge_Max, estimated_cache_size / 1024 > cache_size ? ( estimated_cache_size / 1024 ) : cache_size, //kB
				MUIA_Gauge_Current, estimated_cache_size / 1024,
				MUIA_Gauge_InfoText, GS( PRUNECACHEWIN_GAUGE_USED ),
			End,
			Child, ScaleObject,
				MUIA_Scale_Horiz, TRUE,
			End,
			Child, HGroup,
				Child, Label2( GS( PRUNECACHEWIN_SIZE ) ),
				Child, sl_value = SliderObject,
					MUIA_Slider_Max, estimated_cache_size / 1024 < cache_size ? ( estimated_cache_size / 1024 ) : cache_size,
					MUIA_Slider_Level, estimated_cache_size / 1024 < cache_size ? ( estimated_cache_size / 2048 ) : cache_size / 2,
				End,
			End,
			Child, txt_status = TextObject,
				TextFrame,
				MUIA_Background, MUII_TextBack,
				MUIA_Text_SetMax, FALSE,
				MUIA_Text_SetMin, FALSE,
			End,
			Child, bt_action = button( MSG_PRUNECACHEWIN_START, 0 ),
		End,
	End;

	if( !obj )
	{
		return( 0 );
	}

	data = INST_DATA( cl, obj );

	data->bt_action = bt_action;
	data->ga_fill = ga_fill;
	data->sl_value = sl_value;
	data->txt_status = txt_status;
	data->cache_size = cache_size;

	if( !AttemptSemaphore( &prunecachesem ) )
	{
		/*
		 * There's a running prunecache process
		 */
		DoMethod( obj, MM_PrunecacheWin_Busy, TRUE );
		set( data->txt_status, MUIA_Text_Contents, GS( PRUNECACHEWIN_INPROGRESS ) );
		DoMethod( obj, MM_PrunecacheWin_SetRealTotalSize, estimated_cache_size / 1024 );
	}
	else
	{
		ReleaseSemaphore( &prunecachesem );
		set( data->txt_status, MUIA_Text_Contents, GS( PRUNECACHEWIN_READY ) );
		DoMethod( obj, MM_PrunecacheWin_Busy, FALSE );
	}


	DoMethod( bt_action, MUIM_Notify, MUIA_Pressed, TRUE,
		obj, 1, MM_PrunecacheWin_Process
	);

	DoMethod( obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		MUIV_Notify_Self, 3, MUIM_Set, MUIA_Window_Open, FALSE
	);

	set( obj, MUIA_Window_ActiveObject, bt_action );

	return( (ULONG)obj );
}


DECSMETHOD( PrunecacheWin_Busy )
{
	GETDATA;
	set( data->sl_value, MUIA_Disabled, msg->busy );
	set( data->bt_action, MUIA_Text_Contents, msg->busy ? GS( PRUNECACHEWIN_STOP ) : GS( PRUNECACHEWIN_START ) );

	return( 0 );
}


DECSMETHOD( PrunecacheWin_SetCurrentSize )
{
	GETDATA;

	set( data->ga_fill, MUIA_Gauge_Current, msg->size );
	
	if( msg->all )
	{
		set( data->sl_value, MUIA_Slider_Max, msg->size );
	}

	return( 0 );
}


DECSMETHOD( PrunecacheWin_SetRealTotalSize )
{
	GETDATA;

	if( msg->size > data->cache_size )
	{
		set( data->ga_fill, MUIA_Gauge_Max, msg->size );
	}
	set( data->ga_fill, MUIA_Gauge_Current, msg->size );
	set( data->sl_value, MUIA_Slider_Max, msg->size < data->cache_size ? msg->size : data->cache_size );

	return( 0 );
}

DECSMETHOD( PrunecacheWin_SetTxt )
{
	GETDATA;

	set( data->txt_status, MUIA_Text_Contents, msg->txt );

	return( 0 );
}


DECMETHOD( PrunecacheWin_Process, ULONG )
{
	if( !AttemptSemaphore( &prunecachesem ) )
	{
		/*
		 * Process still running. Signal
		 * it to quit.
		 */
		Forbid();
		while( !AttemptSemaphore( &prunecachesem ) )
		{
			Signal( ( struct Task * )prunecacheproc, SIGBREAKF_CTRL_C );
			Delay( 2 );
		}
		Permit();
		ReleaseSemaphore( &prunecachesem );
		DoMethod( obj, MM_PrunecacheWin_Busy, FALSE );
	}
	else
	{
		GETDATA;

		ReleaseSemaphore( &prunecachesem );

		set( data->bt_action, MUIA_Disabled, TRUE );
		set( data->txt_status, MUIA_Text_Contents, GS( PRUNECACHEWIN_STARTPROC ) );
		DoMethod( obj, MM_PrunecacheWin_Busy, TRUE );
		prune_max_size = getv( data->sl_value, MUIA_Slider_Level ) * 1024;
		startpruneprocess();
		D( db_cacheprune, bug( "prunecacheproc == %ld\n", prunecacheproc ) );
		set( data->bt_action, MUIA_Disabled, FALSE );
	}
	return( 0 );
}


BEGINMTABLE
DEFNEW
DEFSMETHOD( PrunecacheWin_Busy )
DEFSMETHOD( PrunecacheWin_SetCurrentSize )
DEFSMETHOD( PrunecacheWin_SetTxt )
DEFSMETHOD( PrunecacheWin_SetRealTotalSize )
DEFMETHOD( PrunecacheWin_Process )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_prunecachewinclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Window, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "PruneCacheWinClass";
#endif

	return( TRUE );
}

void delete_prunecachewinclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getprunecachewin( void )
{
	return( mcc->mcc_Class );
}


/*
 * Shows an error into the Error Window. Automatically opens it
 * if needed. May be called from any task. url and message may be NULL.
 */
void openprunecachewin( void )
{
	if( !prunecachewin )
	{
		prunecachewin = NewObject( getprunecachewin(), NULL, TAG_DONE );
		
		if ( prunecachewin )
		{
			DoMethod( app, OM_ADDMEMBER, prunecachewin );
		}
	}

	if( prunecachewin )
	{
		set( prunecachewin, MUIA_Window_Open, TRUE );
		DoMethod( prunecachewin, MUIM_Window_ToFront );
	}
	else
	{
		displaybeep();
	}
}

#endif /* USE_NET */
