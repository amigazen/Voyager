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
 * Command class
 * -------------
 * - Group subclass which displays commands selector,
 *   a string and an argument selector
 *
 * © 2000 by VaporWare CVS team <ibcvs@vapor.com>
 * All rights reserved
 *
 * $Id: commandclass.c,v 1.15 2003/07/06 16:51:33 olli Exp $
 *
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <libraries/asl.h>
#include <proto/exec.h>
#endif

/* private */
#include "classes.h"
#include "voyager_cat.h"
#include "init.h"
#include "rexx.h"
#include "prefs.h"
#include "mui_func.h"
#include "methodstack.h"
#include "dos_func.h"
#include "textinput.h"


/*
 * Command modes
 */
enum {
	CMODE_COMMAND,
	CMODE_AMIGADOS,
	CMODE_WORKBENCH,
	CMODE_SCRIPT,
	CMODE_JAVASCRIPT,
	CMODE_AREXX
};


struct Data {
	APTR str_command;
	APTR pop_command;
	APTR pop_template;
	APTR lv_command;
	APTR lv_args;
	APTR lv_template;
	APTR cyc_mode;
	APTR grp_pop;
	APTR pop_args;
	APTR pop_file;
	int mode;	/* current mode to avoid unecessary rebuilds */
	STRPTR backup_command; /* strings to keep a backup if the user cycles through them */
	STRPTR backup_amigados;
	STRPTR backup_workbench;
	STRPTR backup_script;
	STRPTR backup_javascript;
	STRPTR backup_arexx;
};

/*
 * Template list (TOFIX: localize)
 */
struct templatelist {
	STRPTR entry;
	STRPTR description;
};

static const struct templatelist tp_list[] = {
	{"%u", "object's URL"},
	{"%l", "object's link URL"},
	{"%w", "object's window URL"},
	{"%p", "public screen name"},
	{NULL, NULL}
};

/*
 * Allocates area for a string if needed and
 * copies MUIA_String_Contents of obj in it.
 */
void save_backup_string( STRPTR *s, APTR obj )
{
	if( !*s )
	{
		if( !( *s = malloc( VREXX_MAXLENGTH ) ) )
		{
			/* fail silently.. the user won't get a backup */
			return;
		}
	}
	strcpy( *s, ( STRPTR )getv( obj, MUIA_String_Contents ) );
}

/*
 * Restores a backup copy of the string we saved
 * previously using save_backup_string()
 */
void restore_backup_string( STRPTR *s, APTR obj )
{
	if( *s && *s[ 0 ])
	{
		set( obj, MUIA_String_Contents, *s );
	}
	else
	{
		set( obj, MUIA_String_Contents, "" );
	}
}

/*
 * Display hook for the templates
 */
MUI_HOOK( tp_disp, STRPTR *array, struct templatelist *tp )
{
	*array++ = tp->entry;
	*array = tp->description;

	return( 0 );
}

MUI_HOOK( asl_start, APTR popo, ULONG *tagl )
{
	ULONG *p = tagl;

	/*
	 * We put obj into ASLFR_UserData
	 */
	while( *p != TAG_DONE )
	{
		p++;
	}

	*p = ASLFR_UserData;
	p++;
	*p = ( ULONG )h->h_Data;
	p++;
	*p = TAG_DONE;

	return( TRUE );
}

MUI_HOOK( asl_stop, APTR popo, struct FileRequester *fr )
{
	/*
	 * So that we can get it here :)
	 */
	char buf[ 256 ];
	buf[ 0 ] = '\0';

	if( strchr( fr->fr_Drawer, ':' ) )
	{
		strcpy( buf, fr->fr_Drawer );
	}
	else
	{
		/* we have to add the current directory */
		strcpy( buf, myfullpath );
		strcat( buf, fr->fr_Drawer );
	}

	AddPart( buf, fr->fr_File, sizeof( buf ) );

	set( fr->fr_UserData, MA_Command_String_NoReset, buf );
	return( 0 );
}

MUI_HOOK( args_strobj, APTR list, APTR str )
{
	return( ( LONG )DoMethod( h->h_Data, MM_Command_FillArgsList ) );
}
 
MUI_HOOK( args_objstr, APTR list, APTR str )
{
	char buf[ 64 ]; /* should be enough (tm) */
	char *p;
	char *q;
	char *cut = NULL;

	DoMethod( list, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &p );

	/*
	 * Ultra smart ReadArgs-like argument splitter.
	 * /A and /T get inserted, other stuff get a '=' char
	 * after them.
	 */
	strcpy( buf, p );
	q = buf;
	while( *q )
	{
		if( *q == '=' )
		{
			cut = q;
		}
		else
		{
			if( *q == '/' )
			{
				if( *( q + 1 ) == 'S' || *( q + 1 ) == 'T' )
				{
					*q = '\0';
					break;
				}

				if( cut )
				{
					*( cut + 1 ) = '\0';
				}
				else
				{
					*q = '=';
					q++;
					*q = '\0';
				}
				break;
			}
		}
		q++;
	}

	DoMethod( str, MUIM_SetAsString, MUIA_String_Contents, "%s %s", getv( str, MUIA_String_Contents ), buf );
	return( 0 );
}

MUI_HOOK( win, APTR win, APTR popo )
{
	set( win, MUIA_Window_DefaultObject, popo );
	return( 0 );
}

static char * cyc_mode_entries[ 7 ];

DECNEW
{
	struct Data *data;
	struct MUI_Command *mc = rexxcmds;
	APTR pop_command;
	APTR pop_args;
	APTR pop_template;
	APTR str_command;
	APTR lv_command;
	APTR lv_args;
	APTR lv_template;
	APTR cyc_mode;
	APTR grp_pop;
	ULONG i;

	cyc_mode_entries[ 0 ] = GS( COMMANDCLASS_CYCLE_COMMAND );
	cyc_mode_entries[ 1 ] = GS( COMMANDCLASS_CYCLE_AMIGADOS );
	cyc_mode_entries[ 2 ] = GS( COMMANDCLASS_CYCLE_WORKBENCH );
	cyc_mode_entries[ 3 ] = GS( COMMANDCLASS_CYCLE_SCRIPT );
	cyc_mode_entries[ 4 ] = GS( COMMANDCLASS_CYCLE_JS );
	cyc_mode_entries[ 5 ] = GS( COMMANDCLASS_CYCLE_AREXX );

	if( !( obj = ( Object * )DoSuperNew( cl, obj,
		
		Child, HGroup,
			
			Child, cyc_mode = CycleObject,
				MUIA_HorizWeight, 0,
				MUIA_Cycle_Entries, cyc_mode_entries,
				MUIA_CycleChain, 1,
			End,

			Child, grp_pop = HGroup,
	            MUIA_Group_HorizSpacing, 0,
				Child, pop_command = PopobjectObject,
					MUIA_Popobject_WindowHook, &win_hook,
					MUIA_Popstring_Button, PopButton( MUII_PopUp ),
					MUIA_Popobject_Object, lv_command = ListviewObject,
						MUIA_Listview_List, ListObject,
							InputListFrame,
							MUIA_List_AdjustWidth, TRUE,
						End,
					End,
					MUIA_CycleChain, 1,
				End,

				Child, pop_args = PopobjectObject,
					MUIA_Popstring_String, str_command = TextinputObject,
						StringFrame,
						MUIA_String_MaxLen, VREXX_MAXLENGTH,
						MUIA_CycleChain, 1,
					End,
					MUIA_Popstring_Button, PopButton( MUII_PopUp ),
					MUIA_Popobject_StrObjHook, &args_strobj_hook,
					MUIA_Popobject_ObjStrHook, &args_objstr_hook,
					MUIA_Popobject_WindowHook, &win_hook,
					MUIA_Popobject_Object, lv_args = ListviewObject,
						MUIA_Listview_List, ListObject,
							InputListFrame,
						End,
					End,
					MUIA_CycleChain, 1,
				End,

				Child, pop_template = PopobjectObject,
					MUIA_Popstring_Button, TextObject,
						ButtonFrame,
						MUIA_Font, MUIV_Font_Button,
						MUIA_Text_Contents, "T",
						MUIA_Text_SetMax, TRUE,
						MUIA_InputMode, MUIV_InputMode_RelVerify,
						MUIA_Background, MUII_ButtonBack,
					End,

					MUIA_Popobject_WindowHook, &win_hook,
					MUIA_Popobject_Object, lv_template = ListviewObject,
						MUIA_Listview_List, ListObject,
							InputListFrame,
							MUIA_List_AdjustWidth, TRUE,
							MUIA_List_Format, "BAR,",
							MUIA_List_DisplayHook, &tp_disp_hook,
						End,
					End,
					MUIA_CycleChain, 1,
				End,

			End,
		End,
		TAG_MORE, msg->ops_AttrList
	) ) )
	{
		return( 0 );
	}

	/*
	 * Our hooks need the object pointer.
	 * pop_args always exists.
	 */
	args_strobj_hook.h_Data = obj;
	args_objstr_hook.h_Data = obj;

	data = INST_DATA( cl, obj );

	data->str_command = str_command;
	data->pop_command = pop_command;
	data->pop_template = pop_template;
	data->lv_command = lv_command;
	data->lv_args = lv_args;
	data->lv_template = lv_template;
	data->cyc_mode = cyc_mode;
	data->grp_pop = grp_pop;
	data->pop_args = pop_args;


	/*
	 * Fill in the command list
	 */
	while( mc->mc_Name )
	{
		DoMethod( lv_command, MUIM_List_InsertSingle, mc->mc_Name, MUIV_List_Insert_Sorted );
		mc++;
	}

	/*
	 * Fill in the template list
	 */
	for( i = 0; tp_list[ i ].entry; i++ )
	{
		DoMethod( lv_template, MUIM_List_InsertSingle, &tp_list[ i ].entry, MUIV_List_Insert_Bottom );
	}

	/*
	 * Change the popup string according to the command type
	 */
	DoMethod( cyc_mode, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
		obj, 2, MM_Command_ChangeMode, MUIV_TriggerValue
	);

	/*
	 * Double clicking on the lv_command list sets the current string
	 */
	DoMethod( lv_command, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE,
		obj, 1, MM_Command_Select
	);

	/*
	 * And lv_args
	 */
	DoMethod( lv_args, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE,
		pop_args, 2, MUIM_Popstring_Close, TRUE
	);

	/*
	 * And lv_template
	 */
	DoMethod( lv_template, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE,
		obj, 1, MM_Command_Template
	 );

	return( ( ULONG )obj );
}


/*
 * Sets the string depending on what is selected
 * from the command popup ( double click )
 */
DECMETHOD( Command_Select, ULONG )
{
	STRPTR s;
	GETDATA;

	DoMethod( data->lv_command, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &s );
	set( data->str_command, MUIA_String_Contents, s );
	pushmethod( data->pop_command, 2, MUIM_Popstring_Close, TRUE );

	return( 0 );
}


/*
 * Sets the string of the template
 */
DECTMETHOD( Command_Template )
{
	struct templatelist *t;
	GETDATA;

	DoMethod( data->lv_template, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &t );
	DoMethod( data->str_command, MUIM_SetAsString, MUIA_String_Contents, "%s %s", getv( data->str_command, MUIA_String_Contents ), t->entry );
	pushmethod( data->pop_template, 2, MUIM_Popstring_Close, TRUE );

	return( 0 );
}


DECTMETHOD( Command_Remove_Backups )
{
	GETDATA;

	if( data->backup_command )
	{
		data->backup_command[ 0 ] = '\0';
	}

	if( data->backup_amigados )
	{
		data->backup_amigados[ 0 ] = '\0';
	}
	
	if( data->backup_workbench )
	{
		data->backup_workbench[ 0 ] = '\0';
	}
	
	if( data->backup_script )
	{
		data->backup_script[ 0 ] = '\0';
	}
	
	if( data->backup_javascript )
	{
		data->backup_javascript[ 0 ] = '\0';
	}
	
	if( data->backup_arexx )
	{
		data->backup_arexx[ 0 ] = '\0';
	}

	return( 0 );
}

/*
 * Changes the popup depending on the command type
 */
DECSMETHOD( Command_ChangeMode )
{
	GETDATA;
	char buffer[ 14 ]; /* #?.vrx, size is 6 * 2 + 2, TOFIX!! this is not nice */

	D( db_gui, bug( "setting pop to mode %ld\n", msg->mode ) );

	/*
	 * Remove the old one first
	 */
	DoMethod( data->grp_pop, MUIM_Group_InitChange );
	DoMethod( data->grp_pop, OM_REMMEMBER, data->pop_template );
	DoMethod( data->grp_pop, OM_REMMEMBER, data->pop_args );

	if( data->mode == CMODE_COMMAND )
	{
		save_backup_string( ( STRPTR * )&data->backup_command, data->str_command );
		DoMethod( data->grp_pop, OM_REMMEMBER, data->pop_command ); /* we keep this one */
	}
	else
	{
		switch( data->mode )
		{
			case CMODE_AMIGADOS:
				save_backup_string( ( STRPTR * )&data->backup_amigados, data->str_command );
				break;

			case CMODE_WORKBENCH:
				save_backup_string( ( STRPTR * )&data->backup_workbench, data->str_command );
				break;
			
			case CMODE_SCRIPT:
				save_backup_string( ( STRPTR * )&data->backup_script, data->str_command );
				break;
			
			case CMODE_JAVASCRIPT:
				save_backup_string( ( STRPTR * )&data->backup_javascript, data->str_command );
				break;
			
			case CMODE_AREXX:
				save_backup_string( ( STRPTR * )&data->backup_arexx, data->str_command );
				break;
		}
		DoMethod( data->grp_pop, OM_REMMEMBER, data->pop_file );
		MUI_DisposeObject( data->pop_file );

	}

	/*
	 * And create (if needed) then add the new one
	 */
	switch( msg->mode )
	{
		case CMODE_COMMAND:
			/* no need to create since that one is kept */
			restore_backup_string( ( STRPTR * )&data->backup_command, data->str_command );
			DoMethod( data->grp_pop, OM_ADDMEMBER, data->pop_command );
			break;

		case CMODE_AMIGADOS:
			data->pop_file = PopaslObject,
				MUIA_Popstring_Button, PopButton( MUII_PopFile ),
				MUIA_Popasl_StartHook, &asl_start_hook,
				MUIA_Popasl_StopHook, &asl_stop_hook,
				ASLFR_TitleText, GS( COMMANDCLASS_AMIGADOS_ASLTITLE ),
				ASLFR_RejectIcons, TRUE,
			End;
			
			asl_start_hook.h_Data = obj;

			if( !data->pop_file )
			{
				exit( 0 ); //TOFIX!! be smarter somewhat..
			}

			restore_backup_string( ( STRPTR * )&data->backup_amigados, data->str_command );
			DoMethod( data->grp_pop, OM_ADDMEMBER, data->pop_file );
			break;

		case CMODE_WORKBENCH:
			data->pop_file = PopaslObject,
				MUIA_Popstring_Button, PopButton( MUII_PopFile ),
				MUIA_Popasl_StartHook, &asl_start_hook,
				MUIA_Popasl_StopHook, &asl_stop_hook,
				ASLFR_TitleText, GS( COMMANDCLASS_WORKBENCH_ASLTITLE ),
				ASLFR_RejectIcons, TRUE,
			End;
			
			asl_start_hook.h_Data = obj;

			if( !data->pop_file )
			{
				exit( 0 ); //TOFIX!! be smarter somewhat..
			}

			restore_backup_string( ( STRPTR * )&data->backup_workbench, data->str_command );
			DoMethod( data->grp_pop, OM_ADDMEMBER, data->pop_file );
			break;

		case CMODE_SCRIPT:
			data->pop_file = PopaslObject,
				MUIA_Popstring_Button, PopButton( MUII_PopFile ),
				MUIA_Popasl_StartHook, &asl_start_hook,
				MUIA_Popasl_StopHook, &asl_stop_hook,
				ASLFR_TitleText, GS( COMMANDCLASS_SCRIPT_ASLTITLE ),
				ASLFR_RejectIcons, TRUE,
				ASLFR_InitialDrawer, "S:",
			End;
			
			asl_start_hook.h_Data = obj;
			
			if( !data->pop_file )
			{
				exit( 0 ); //TOFIX!! be smarter somewhat..
			}
			
			restore_backup_string( ( STRPTR * )&data->backup_script, data->str_command );
			DoMethod( data->grp_pop, OM_ADDMEMBER, data->pop_file );
			break;

		case CMODE_JAVASCRIPT:
			ParsePatternNoCase( "#?.js", buffer, 12 );

			data->pop_file = PopaslObject,
				MUIA_Popstring_Button, PopButton( MUII_PopFile ),
				MUIA_Popasl_StartHook, &asl_start_hook,
				MUIA_Popasl_StopHook, &asl_stop_hook,
				ASLFR_TitleText, GS( COMMANDCLASS_JAVASCRIPT_ASLTITLE ),
				ASLFR_RejectIcons, TRUE,
				ASLFR_DoPatterns, TRUE,
				ASLFR_AcceptPattern, buffer,
				ASLFR_InitialDrawer, "Rexx",
			End;
			
			asl_start_hook.h_Data = obj;
			
			if( !data->pop_file )
			{
				exit( 0 ); //TOFIX!! be smarter somewhat..
			}

			restore_backup_string( ( STRPTR * )&data->backup_javascript, data->str_command );
			DoMethod( data->grp_pop, OM_ADDMEMBER, data->pop_file );
			break;
 
		case CMODE_AREXX:
			ParsePatternNoCase( "#?.vrx", buffer, 14 );

			data->pop_file = PopaslObject,
				MUIA_Popstring_Button, PopButton( MUII_PopFile ),
				MUIA_Popasl_StartHook, &asl_start_hook,
				MUIA_Popasl_StopHook, &asl_stop_hook,
				ASLFR_TitleText, GS( COMMANDCLASS_AREXX_ASLTITLE ),
				ASLFR_RejectIcons, TRUE,
				ASLFR_DoPatterns, TRUE,
				ASLFR_AcceptPattern, buffer,
				ASLFR_InitialDrawer, "Rexx",
			End;
			
			asl_start_hook.h_Data = obj;
			
			if( !data->pop_file )
			{
				exit( 0 ); //TOFIX!! be smarter somewhat..
			}

			restore_backup_string( ( STRPTR * )&data->backup_arexx, data->str_command );
			DoMethod( data->grp_pop, OM_ADDMEMBER, data->pop_file );
			break;
	}

	DoMethod( data->grp_pop, OM_ADDMEMBER, data->pop_args );
	DoMethod( data->grp_pop, OM_ADDMEMBER, data->pop_template );
	DoMethod( data->grp_pop, MUIM_Group_ExitChange );

	data->mode = msg->mode;

	return( 0 );
}


/*
 * Fills in the args list with the arguments
 * used by the command in the string
 */

DECMETHOD( Command_FillArgsList, ULONG )
{
	struct MUI_Command *mc = rexxcmds; 
	int len;
	int l = 0;
	char *p;
	GETDATA;
	STRPTR s = ( STRPTR )getv( data->str_command, MUIA_String_Contents );

	DoMethod( data->lv_args, MUIM_List_Clear );

	/*
	 * Search the command in the string
	 */
	len = strlen( s );
	p = s;

	while( *p )
	{
		if( *p == ' ' )
		{
			len = l;
			break;
		}

		l++;
		p++;
	}

	while( mc->mc_Name )
	{
		if( len == strlen( mc->mc_Name ) && !strnicmp( mc->mc_Name, s, len ) )
		{
			break;
		}
		mc++;
	}

	if( mc->mc_Name )
	{
		if( mc->mc_Parameters )
		{
			/* fill-in */
			char *t;
			char *buf;

			if( buf = malloc( strlen( mc->mc_Template ) ) )
			{
				strcpy( buf, mc->mc_Template );

				t = strtok( buf, "," );
				while( t )
				{
					DoMethod( data->lv_args, MUIM_List_InsertSingle, t, MUIV_List_Insert_Bottom );
					t = strtok( NULL, "," );
				}
			}
		}
	}
	return( TRUE );
}


DECGET
{
	GETDATA;

	switch( msg->opg_AttrID )
	{
		case MA_Command_String:
			*msg->opg_Storage = ( ULONG )getv( data->str_command, MUIA_String_Contents );
			return( TRUE );
			break;

		case MA_Command_Mode:
			*msg->opg_Storage = ( ULONG )getv( data->cyc_mode, MUIA_Cycle_Active ) + BFUNC_COMMAND;
			return( TRUE );
			break;
	}
	return( DOSUPER );
}


DECSET
{
	struct TagItem *tag, *tagp = msg->ops_AttrList;
	GETDATA;

	while( tag = NextTagItem( &tagp ) ) switch( tag->ti_Tag )
	{
		case MA_Command_String:
			DoMethod( obj, MM_Command_Remove_Backups );
			/* fallthrough */
		case MA_Command_String_NoReset:
			set( data->str_command, MUIA_String_Contents, ( STRPTR )tag->ti_Data );
			break;
	
		case MA_Command_Mode:
			DoMethod( obj, MM_Command_Remove_Backups );
			set( data->cyc_mode, MUIA_Cycle_Active, ( ULONG )tag->ti_Data - BFUNC_COMMAND );
			break;
	}
	return( DOSUPER );
}


DECDISPOSE
{
	GETDATA;

	if( data->backup_command )
	{
		free( data->backup_command );
	}

	if( data->backup_amigados )
	{
		free( data->backup_amigados );
	}
	
	if( data->backup_workbench )
	{
		free( data->backup_workbench );
	}
	
	if( data->backup_script )
	{
		free( data->backup_script );
	}
	
	if( data->backup_javascript )
	{
		free( data->backup_javascript );
	}

	if( data->backup_arexx )
	{
		free( data->backup_arexx );
	}

	return( DOSUPER );
}


BEGINMTABLE
DEFNEW
DEFSET
DEFGET
DEFMETHOD( Command_Select )
DEFSMETHOD( Command_ChangeMode )
DEFMETHOD( Command_FillArgsList )
DEFTMETHOD( Command_Template )
DEFTMETHOD( Command_Remove_Backups )
DEFDISPOSE
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_commandclass( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, MUIC_Group, NULL, sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "CommandClass";
#endif

	return( TRUE );
}

void delete_commandclass( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getcommandclass( void )
{
	return( mcc->mcc_Class );
}

