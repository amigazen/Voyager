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
** $Id: js_stb_cdplayer.c,v 1.42 2003/07/06 16:51:33 olli Exp $
*/

/* Don't show this on public JS object listing: NO_PUBLIC_LISTING */

/*

	The Javascript "STB.CDPlayer" object offers
	functionality for controlling the CaOS CD player device.

	Functions:

		play()      - continues playing at the current qcode (after ffw/rew)
		play(track) - play given track
		ffwd()      - start seeking forward
		frew()      - start seeking backward
		eject()     - eject disk
		load()	    - load disk
		stop()	    - stop playing
		pause()	    - pause playing

	Properties:

		CD information:

		numtracks (real)	- number of tracks on disk
		tracks (obj)		- array with information about tracks

		Currently playing:

		track (real)		- currently playing track number
		trackMin (real)		- offset into track
		trackSec (real)		- offset into track
		trackFrame (real)	- offset into track
		diskMin (real)		- offset into disk
		diskSec (real)		- offset into disk
		diskFrame (real)	- offset into disk

	Event handlers:

		ondiskchange		- function to be called when disk is inserted
							  or removed. N.B.: This MUST be specified
							  in the <BODY> tag!

	Tracks object:

		the "tracks" object property mentioned above is actually an
		array with an entry for each track (starting at "1")

		tracks[track].track
			this tracks' number

		tracks[track].trackMinutes
		tracks[track].trackSeconds
			length of track

		tracks[track].trackStartMinutes
		tracks[track].trackStartSeconds
			start of track on disk

		tracks[track].trackTitle
		tracks[track].trackPerformer
		tracks[track].trackSongWriter
		tracks[track].trackComposer
		tracks[track].trackArranger
		tracks[track].trackMessage
			CD-Text info of track (if available)

		In addition to the track entries, the object holds the
		following per-disk information:

		tracks.diskMinutes
		tracks.diskSeconds
			total disk length

		tracks.diskTracks
			number of tracks (equal to CDPlayer.numtracks)

		tracks.cdTextAvailable
			boolean flag whether the disk had CD-Text info

		tracks.diskTitle
		tracks.diskPerformer
		tracks.diskSongWriter
		tracks.diskComposer
		tracks.diskArranger
		tracks.diskMessage
			CD-Text info of disk (if available)

*/

#include "voyager.h"
#include "classes.h"
#include "htmlclasses.h"
#include "win_func.h"
#include "js.h"
#include "copyright.h"
#include "mui_func.h"

#ifdef MBX

#include <system/blockdevice.h>
#include <igs5050_lib_calls.h>         
#include <ide_drv_calls.h>

static Igs5050Data_p IGSBase;
static MsgQueue_p queue, change;
static DrvIOReqStd_p ior;
static union CDTOC myTOC[100];
static CDText_s myCDT[100];
static QCode_s myQCode;
static CDPlay_s myPlay;
static DriveGeometry_s	myGeometry;
static DiskChangeMsg_p pChangeMsg;
static ChangeData_s data;
static int devopen;
static int numtracks;
static int numcdtexttracks;
static int firsttrack;
static int sigbit;
static int toc_read, cdtext_read;
static struct MUI_InputHandlerNode ihn;

struct Data 
{
	int dummy;
};

static void closecd( void )
{
	
	D( db_js, bug( "Close CD Device 1\n" ) );

	if( !devopen )
		return;

	D( db_js, bug( "Close CD Device 2\n" ) );

	DoMethod( app, MUIM_Application_RemInputHandler, &ihn );

	// remove diskchange
	ior->ior_Command = CMD_BLK_REMCHANGEMSG;
	ior->ior_DataPtr = &data;
	ior->ior_Length = sizeof(data);
	ior->ior_Error = IOERR_OK;
	ior->ior_Actual = 0;
	DoIO((DrvIOReq_p)ior);
	
	// everything fine, quit now
	CloseDriver( (DrvIOReq_p)ior );
	DeleteStdIO( ior );
	DeleteMsgQueue( queue, TRUE);
	DeleteMsgQueue( change, TRUE);

	CloseModule( (Module_p)IGSBase );

	devopen = FALSE;
}

static int opencd( void )
{
	DrvIOReqStd_p ior2;
	MsgQueue_p queue2;
	int unitcount = 0;
	UDWORD *unittable = NULL;
	int unit, i;
	int error, actual;

	D( db_js, bug( "Open CD Device 1\n" ) );

	if( devopen )
		return( TRUE );

	D( db_js, bug( "Open CD Device 2\n" ) );

	IGSBase = (Igs5050Data_p)OpenModule(IGS5050NAME, IGS5050VERSION );
						
	toc_read = FALSE;

	unit = UNIT_DUMMY; // dummy unit

	// we use 2 IORequests to avoid the Expunge of the device
	if ( (change = CreateMsgQueue( "ChangeMsgQueue", 0, TRUE)) )
	{
		if ( (queue = CreateMsgQueue( "VDevs", 0, TRUE)) )
		{
			if ( (ior = CreateStdIO(  queue )) )
			{
				if ( (queue2 = CreateMsgQueue( "VDevs2", 0, TRUE)) )
				{
					if ( (ior2 = CreateStdIO(  queue2 )) )
					{
						if (!OpenDriver(IDENAME, unit, (DrvIOReq_p) ior2, 0))
						{
							// ask for the number of active units
							ior2->ior_DataPtr = NULL;
							ior2->ior_Length = 0;
							ior2->ior_Command = CMD_UNITCOUNT;
							ior2->ior_Error = IOERR_OK;
							ior2->ior_Actual = 0;
							DoIO((DrvIOReq_p)ior2);
							if (ior2->ior_Error >= IOERR_OK)
							{
								unitcount = ior2->ior_Actual;
								if ((unittable = AllocMem(sizeof(UDWORD) * unitcount, MEMF_CLEAR)) != NULL)
								{
									// ask for the unit numbers
									ior2->ior_DataPtr = unittable;
									ior2->ior_Length = unitcount;
									ior2->ior_Command = CMD_UNITTABLE;
									ior2->ior_Error = IOERR_OK;
									ior2->ior_Actual = 0;
									DoIO((DrvIOReq_p)ior2);
								}
							}
							error = ior2->ior_Error;
				
							if ((error >= 0) && (unittable != NULL) && (unitcount > 0))
							{
								actual = ior2->ior_Actual;
								// ask every single unit if it is a CDRom drive
								for(i = 0; i < MIN(actual, unitcount); i++)
								{
									if (!OpenDriver(IDENAME, unittable[i], (DrvIOReq_p) ior, 0))
									{
										// ask for the Type
										ior->ior_DataPtr = &myGeometry;
										ior->ior_Length = sizeof(DriveGeometry_s);
										ior->ior_Command = CMD_BLK_GETGEOMETRY;
										ior->ior_Error = IOERR_OK;
										ior->ior_Actual = 0;
										DoIO((DrvIOReq_p)ior);
										if (ior->ior_Error >= IOERR_OK)
										{
											if (myGeometry.dg_DeviceType == ATAPI_CD_ROM_DEVICE)
											{
												unit = unittable[i]; // remember the unit-number
												//i = unitcount; // finish the loop
												break; // go out of here, leave the device open
											}
										}
										CloseDriver(  (DrvIOReq_p)ior );
									}
								}
							}
	      		
							// close the dummy device
							CloseDriver(  (DrvIOReq_p)ior2 );
						}
						DeleteStdIO(  ior2 );
					}
					DeleteMsgQueue(  queue2, TRUE);
				}

				if (unittable != NULL)
					FreeMem(unittable, sizeof(UDWORD) * unitcount);
				
				if (unit != UNIT_DUMMY)
				{
					// we left it open
					//if (!OpenDriver(IDENAME, unit, (DrvIOReq_p) ior, 0))
					{
						// setup diskchange
						sigbit = SetMsgQueueSignal(change, FindProcess(0), -1);
		      
						data.cd_Queue = change;
						data.cd_ProcessID = 0; //FIXME: our ProcessID
						data.cd_UserData = NULL; // FIXME: in case we need some
						ior->ior_Command = CMD_BLK_ADDCHANGEMSG;
						ior->ior_DataPtr = &data;
						ior->ior_Length = sizeof(data);
						ior->ior_Error = IOERR_OK;
						ior->ior_Actual = 0;
						DoIO((DrvIOReq_p)ior);
	
						devopen = TRUE;

						ihn.ihn_stuff.ihn_sigs = 1L << sigbit;

						ihn.ihn_Object = app;
						ihn.ihn_Method = MM_JS_CDPlayer_DiskChange;

						DoMethod( app, MUIM_Application_AddInputHandler, &ihn );

						D( db_js, bug( "CD Device opened properly\n" ) );
					}
				}
			}
		}
	}

	return( devopen );
}

static int readqcode( void )
{
	if( !opencd() )
		return( FALSE );

	ior->ior_Command = CMD_CD_QCODEMSF;
	ior->ior_DataPtr = &myQCode;
	ior->ior_Length = sizeof(myQCode);
	ior->ior_Error = IOERR_OK;
	ior->ior_Actual = 0;
	DoIO((DrvIOReq_p)ior);
	if (ior->ior_Error >= IOERR_OK)
	{
		if (ior->ior_Actual < 0x13)
		{
			D( db_js, bug( "returning true\n" ) );
			return( TRUE );
		}
	}
	D( db_js, bug( "returning false\n" ) );
	return( FALSE );
}

static int readtoc( void )
{
	D( db_js, bug( "toc read == %ld\n", toc_read ) );

	if( toc_read )
		return( TRUE );

	D( db_js, bug( "Reading TOC...\n" ) );

	if( !opencd() )
		return( FALSE );

	numtracks = -1;
	cdtext_read = FALSE;

	memset(&myTOC[0], 0, sizeof(myTOC));
	memset(&myCDT, 0, sizeof(myCDT));

	ior->ior_Command = CMD_CD_TOCMSF;
	ior->ior_DataPtr = &myTOC[0];
	ior->ior_Length = sizeof(myTOC);
	ior->ior_Error = IOERR_OK;
	ior->ior_Actual = 0;
	DoIO((DrvIOReq_p)ior);

	D( db_js, bug( "CMD_CD_TOCMSF error %ld, actual %ld\n", ior->ior_Error, ior->ior_Actual ) );

	if(ior->ior_Error < 0)
		return( FALSE );

	//numtracks = ior->ior_Actual / sizeof(union CDTOC);

	firsttrack = myTOC[0].toc_Summary.tocs_FirstTrack;
	numtracks = myTOC[0].toc_Summary.tocs_LastTrack - firsttrack + 1;

	D( db_js, bug( "numtracks %ld\n", numtracks ) );

	// Also read CDText
	ior->ior_Command = CMD_CD_READCDTEXT;
	ior->ior_DataPtr = &myCDT[0];
	ior->ior_Length = sizeof(myCDT);
	ior->ior_Error = IOERR_OK;
	ior->ior_Actual = 0;
	DoIO((DrvIOReq_p)ior);

	D( db_js, bug( "CMD_CD_READCDTEXT error %ld, actual %ld\n", ior->ior_Error, ior->ior_Actual ) );

	if( ior->ior_Error >= IOERR_OK)
	{
		numcdtexttracks = ( ior->ior_Actual / sizeof( myCDT[0] ) ) - 1;
		if( numcdtexttracks > 0 )
			cdtext_read = TRUE;
	}

	toc_read = TRUE;
	return( TRUE );
}

static int simplecmd( int cmd, int length )
{
	if( !opencd() )
		return( -1 );

	ior->ior_Command = cmd;
	ior->ior_DataPtr = NULL;
	ior->ior_Length = length;
	ior->ior_Error = IOERR_OK;
	ior->ior_Actual = 0;
	DoIO((DrvIOReq_p)ior);

	D( db_js, bug( "simplecmd(%ld,%ld) -> %ld\n", cmd, length, ior->ior_Error ) );

	return( ior->ior_Error );
}

BEGINPTABLE
DPROP( track, 		real )
DPROP( trackMin, 	real )
DPROP( trackSec, 	real )
DPROP( trackFrame, 	real )
DPROP( diskMin, 	real )
DPROP( diskSec, 	real )
DPROP( diskFrame, 	real )
DPROP( numtracks, 	real )
DPROP( ffwd, 		funcptr )
DPROP( frev, 		funcptr )
DPROP( play, 		funcptr )
DPROP( eject, 		funcptr )
DPROP( load, 		funcptr )
DPROP( stop, 		funcptr )
DPROP( pause, 		funcptr )
DPROP( refresh, 	funcptr )
DPROP( tracks, 		obj )
ENDPTABLE

DECNEW
{
	obj = DoSuperNew( cl, obj,
		MA_JS_ClassName, "CDPlayer",
		TAG_MORE, msg->ops_AttrList
	);

	return( (ULONG)obj );
}

static APTR buildtrackinfo( void )
{
	APTR arr;
	int c;
	struct MinList *cpl;

	arr = JSNewObject( getjs_array(),
		MA_JS_Object_TerseArray, TRUE,
		TAG_DONE
	);
	if( !arr )
		return( NULL );

	get( arr, MA_JS_Object_CPL, &cpl );

	for( c = 0; c < numtracks; c++ )
	{
		APTR o;
		char name[ 16 ];
		struct MinList *cplo;
		int minutes, seconds;

		o = JSNewObject( getjs_object(),
			TAG_DONE
		);
		if( !o )
			break;

		get( o, MA_JS_Object_CPL, &cplo );

		cp_setreal( cplo, "track", c + 1 );

		D( db_js, bug( "setting properties for track %ld\n", c ) );

		if( c == numtracks - 1 )
		{
			minutes = myTOC[0].toc_Summary.tocs_LeadOut.lsnmsf_MSF.rmsf_Minute - myTOC[firsttrack+c].toc_Entry.toce_Position.lsnmsf_MSF.rmsf_Minute;
			seconds = myTOC[0].toc_Summary.tocs_LeadOut.lsnmsf_MSF.rmsf_Second - myTOC[firsttrack+c].toc_Entry.toce_Position.lsnmsf_MSF.rmsf_Second;
		}
		else
		{
			minutes = myTOC[firsttrack+c+1].toc_Entry.toce_Position.lsnmsf_MSF.rmsf_Minute - myTOC[firsttrack+c].toc_Entry.toce_Position.lsnmsf_MSF.rmsf_Minute;
			seconds = myTOC[firsttrack+c+1].toc_Entry.toce_Position.lsnmsf_MSF.rmsf_Second - myTOC[firsttrack+c].toc_Entry.toce_Position.lsnmsf_MSF.rmsf_Second;
		}
		if( seconds < 0 )
		{
			minutes--;
			seconds += 60;
		}
		cp_setreal( cplo, "trackMinutes", minutes );
		cp_setreal( cplo, "trackSeconds", seconds );
		cp_setreal( cplo, "trackStartMinutes", myTOC[firsttrack+c].toc_Entry.toce_Position.lsnmsf_MSF.rmsf_Minute );
		cp_setreal( cplo, "trackStartSeconds", myTOC[firsttrack+c].toc_Entry.toce_Position.lsnmsf_MSF.rmsf_Second );

		if( cdtext_read )
		{
			cp_setstr( cplo, "trackTitle", myCDT[c+1].cdt_Title, -1 );
			cp_setstr( cplo, "trackPerformer", myCDT[c+1].cdt_Performer, -1 );
			cp_setstr( cplo, "trackSongWriter", myCDT[c+1].cdt_SongWriter, -1 );
			cp_setstr( cplo, "trackComposer", myCDT[c+1].cdt_Composer, -1 );
			cp_setstr( cplo, "trackArranger", myCDT[c+1].cdt_Arranger, -1 );
			cp_setstr( cplo, "trackMessage", myCDT[c+1].cdt_Message, -1 );
		}

		sprintf( name, "%d", c + 1 );
		cp_set( cpl, name, o );
	}

	cp_setreal( cpl, "cdTextAvailable", cdtext_read ? 1 : 0 );
	if( cdtext_read )
	{
		cp_setstr( cpl, "diskTitle", myCDT[0].cdt_Title, -1 );
		cp_setstr( cpl, "diskPerformer", myCDT[0].cdt_Performer, -1 );
		cp_setstr( cpl, "diskSongWriter", myCDT[0].cdt_SongWriter, -1 );
		cp_setstr( cpl, "diskComposer", myCDT[0].cdt_Composer, -1 );
		cp_setstr( cpl, "diskArranger", myCDT[0].cdt_Arranger, -1 );
		cp_setstr( cpl, "diskMessage", myCDT[0].cdt_Message, -1 );
	}

	cp_setreal( cpl, "diskMinutes", myTOC[0].toc_Summary.tocs_LeadOut.lsnmsf_MSF.rmsf_Minute );
	cp_setreal( cpl, "diskSeconds", myTOC[0].toc_Summary.tocs_LeadOut.lsnmsf_MSF.rmsf_Second );
	cp_setreal( cpl, "diskTracks", numtracks );

	return( arr );
}

DECSMETHOD( JS_GetProperty )
{
	struct propt *pt = findprop( ptable, msg->propname );

	if( !pt )
		return( DOSUPER );

	if( pt->type == expt_funcptr )
	{
		storefuncprop( msg, -pt->id );
		return( TRUE );
	}

	switch( pt->id )
	{
		case JSPID_track:
			if( !readqcode() )
			{
				storerealprop( msg, -1 );
			}
			else
			{
				storerealprop( msg, myQCode.qc_Track );
			}
			return( TRUE );

		case JSPID_trackMin:
			if( !readqcode() )
			{
				storerealprop( msg, -1 );
			}
			else
			{
				storerealprop( msg, myQCode.qc_TrackPosition.lsnmsf_MSF.rmsf_Minute );
			}
			return( TRUE );

		case JSPID_trackSec:
			if( !readqcode() )
			{
				storerealprop( msg, -1 );
			}
			else
			{
				storerealprop( msg, myQCode.qc_TrackPosition.lsnmsf_MSF.rmsf_Second );
			}
			return( TRUE );

		case JSPID_trackFrame:
			if( !readqcode() )
			{
				storerealprop( msg, -1 );
			}
			else
			{
				storerealprop( msg, myQCode.qc_TrackPosition.lsnmsf_MSF.rmsf_Frame );
			}
			return( TRUE );

		case JSPID_diskMin:
			if( !readqcode() )
			{
				storerealprop( msg, -1 );
			}
			else
			{
				storerealprop( msg, myQCode.qc_DiskPosition.lsnmsf_MSF.rmsf_Minute );
			}
			return( TRUE );

		case JSPID_diskSec:
			if( !readqcode() )
			{
				storerealprop( msg, -1 );
			}
			else
			{
				storerealprop( msg, myQCode.qc_DiskPosition.lsnmsf_MSF.rmsf_Second );
			}
			return( TRUE );

		case JSPID_diskFrame:
			if( !readqcode() )
			{
				storerealprop( msg, -1 );
			}
			else
			{
				storerealprop( msg, myQCode.qc_DiskPosition.lsnmsf_MSF.rmsf_Frame );
			}
			return( TRUE );

		case JSPID_numtracks:
			readtoc();
			storerealprop( msg, numtracks );
			return( TRUE );

		case JSPID_tracks:
			{
				if( readtoc() )
				{
					APTR io = buildtrackinfo();
					storeobjprop( msg, io );
				}
				else
				{
					storeobjprop( msg, NULL );
				}
			}
			return( TRUE );
	}

	return( FALSE );
}

DECSMETHOD( JS_HasProperty )
{
	struct propt *pt;

	if( pt = findprop( ptable, msg->propname ) )
		return( (ULONG)pt->type );

	return( DOSUPER );
}

DECSMETHOD( JS_CallMethod )
{
	static double rval;

	switch( msg->pid )
	{
		case JSPID_refresh:
			toc_read = FALSE;
			return( TRUE );

		case JSPID_stop:
			rval = simplecmd( CMD_CD_STOP, 0 );
			*msg->typeptr = expt_real;
			*msg->dataptr = &rval;
			return( TRUE );

		case JSPID_eject:
			rval = simplecmd( CMD_BLK_EJECT, 1 );
			*msg->typeptr = expt_real;
			*msg->dataptr = &rval;
			return( TRUE );

		case JSPID_load:
			rval = simplecmd( CMD_BLK_EJECT, 0 );
			*msg->typeptr = expt_real;
			*msg->dataptr = &rval;
			return( TRUE );

		case JSPID_pause:
			rval = simplecmd( CMD_CD_PAUSE, 1 );
			*msg->typeptr = expt_real;
			*msg->dataptr = &rval;
			return( TRUE );

		case JSPID_ffwd:
			rval = simplecmd( CMD_CD_SEARCH, CDMODE_FFWD );
			*msg->typeptr = expt_real;
			*msg->dataptr = &rval;
			return( TRUE );

		case JSPID_frev:
			rval = simplecmd( CMD_CD_SEARCH, CDMODE_FREV );
			*msg->typeptr = expt_real;
			*msg->dataptr = &rval;
			return( TRUE );

		case JSPID_play:
			{
				rval = -1;
				if( opencd() )
				{
					if( readtoc() )
					{
						if( msg->argcnt-- > 0 )
						{
							int track;

							track = exprs_pop_as_real( msg->es );
							if( track < 1 )
								track = 1;
							else if( track > numtracks )
								track = numtracks;

							track--;

							D( db_js, bug( "Playing track %ld (offset 0!)\n", track ) );

							myPlay.cdp_Start.lsnmsf_MSF.rmsf_Minute = myTOC[firsttrack+track].toc_Entry.toce_Position.lsnmsf_MSF.rmsf_Minute;
							myPlay.cdp_Start.lsnmsf_MSF.rmsf_Second = myTOC[firsttrack+track].toc_Entry.toce_Position.lsnmsf_MSF.rmsf_Second;
							myPlay.cdp_Start.lsnmsf_MSF.rmsf_Frame = myTOC[firsttrack+track].toc_Entry.toce_Position.lsnmsf_MSF.rmsf_Frame;
						}
						else
						{
							D( db_js, bug( "Playing from current position\n" ) );

							readqcode();
					  		myPlay.cdp_Start.lsnmsf_MSF.rmsf_Minute = myQCode.qc_DiskPosition.lsnmsf_MSF.rmsf_Minute;
					  		myPlay.cdp_Start.lsnmsf_MSF.rmsf_Second = myQCode.qc_DiskPosition.lsnmsf_MSF.rmsf_Second;
					  		myPlay.cdp_Start.lsnmsf_MSF.rmsf_Frame = myQCode.qc_DiskPosition.lsnmsf_MSF.rmsf_Frame;
						}

				  		myPlay.cdp_Start.lsnmsf_MSF.rmsf_Reserved = 0;
				  		myPlay.cdp_End.lsnmsf_MSF.rmsf_Reserved = 0;
				  		myPlay.cdp_End.lsnmsf_MSF.rmsf_Minute = myTOC[0].toc_Summary.tocs_LeadOut.lsnmsf_MSF.rmsf_Minute;
				  		myPlay.cdp_End.lsnmsf_MSF.rmsf_Second = myTOC[0].toc_Summary.tocs_LeadOut.lsnmsf_MSF.rmsf_Second;
				  		myPlay.cdp_End.lsnmsf_MSF.rmsf_Frame = myTOC[0].toc_Summary.tocs_LeadOut.lsnmsf_MSF.rmsf_Frame;
				  		ior->ior_Command = CMD_CD_PLAYMSF;
				  		ior->ior_DataPtr = &myPlay;
				  		ior->ior_Length = sizeof(myPlay);
				  		ior->ior_Error = IOERR_OK;
				  		ior->ior_Actual = 0;
				  		DoIO((DrvIOReq_p)ior);
						igs_WriteAC97Mixer( AC97AUXVOL, 0x0000 );
						rval = ior->ior_Error;
					}
				}
				*msg->typeptr = expt_real;
				*msg->dataptr = &rval;
				return( TRUE );
			}
	}
	return( DOSUPER );
}

DS_LISTPROP

BEGINMTABLE
DEFNEW
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_HasProperty )
DEFSMETHOD( JS_CallMethod )
DEFSMETHOD( JS_ListProperties )
ENDMTABLE

static struct MUI_CustomClass *mcc;

void js_stb_gotdiskchange( void )
{
	pChangeMsg = (DiskChangeMsg_p)GetMsg(change);
	if( pChangeMsg )
		ReplyMsg((Message_p)pChangeMsg);
	toc_read = FALSE;

	// Actually fire off the diskchange event to the windows
	doallwins( MM_JS_CDPlayer_DiskChange, 0 );
}

int create_js_stb_cdplayer( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_object_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "JS_stb_cdplayerClass";
#endif

	return( TRUE );
}

void delete_js_stb_cdplayer( void )
{
	closecd();

	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getjs_stb_cdplayer( void )
{
	return( mcc->mcc_Class );
}

#endif /* MBX */
