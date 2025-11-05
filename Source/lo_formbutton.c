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
**
** $Id: lo_formbutton.c,v 1.10 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"
#include "classes.h"
#include "htmlclasses.h"
#include "malloc.h"
#include "mui_func.h"

static struct MUI_CustomClass *lcc;

struct Data {
	APTR formobject;
	char *name;
	char *id;
	char *value;
	int type;
	int meactive;
};

static int doset( struct Data *data, APTR obj, struct TagItem *tags, int *sendup)
{
	struct TagItem *tag;

#define KILLT tag->ti_Tag = TAG_IGNORE

	while( ( tag = NextTagItem( &tags ) ) ) switch( (int)tag->ti_Tag )
	{
		case MA_Layout_FormElement_Name:
			if( tag->ti_Data )
				data->name = (char *)strdup( (char *)tag->ti_Data ); /* TOFIX */
			KILLT;
			break;

		case MA_Layout_FormElement_DOMID:
			if( tag->ti_Data )
				data->id = (char *)strdup( (char *)tag->ti_Data ); /* TOFIX */
			KILLT;
			break;

		case MA_Layout_FormElement_Value:
			if( tag->ti_Data )
			{
				data->value = (char *)strdup( (char*)tag->ti_Data ); /* TOFIX */
				set( obj, MUIA_Text_Contents, tag->ti_Data );
			}
			KILLT;
			break;

		case MA_Layout_FormElement_Form:
			data->formobject = (APTR)tag->ti_Data;
			KILLT;
			break;

		case MA_Layout_FormButton_Type:
			data->type = tag->ti_Data;
			KILLT;
			break;

		case (int)MUIA_Pressed:
			if( !tag->ti_Data && data->formobject )
			{
				if( data->type == formbutton_submit )
				{
					data->meactive = TRUE;
					DoMethod( data->formobject, MM_Layout_Form_Submit, obj );
					data->meactive = FALSE;
				}
				else if( data->type == formbutton_reset )
				{
					DoMethod( data->formobject, MM_Layout_Form_Reset );
				}
			}
//			KILLT;
			break;

		default:
			if( sendup )
				*sendup = TRUE;
			break;
	}

	return( 0 );
}

DECCONST
{
	struct Data *data;

	obj = DoSuperNew( cl, obj,
		MUIA_Frame, MUIV_Frame_Button,
		MUIA_Background, MUII_ButtonBack,
		MUIA_InputMode, MUIV_InputMode_RelVerify,
		MA_Layout_Group_UseMUIBackground, TRUE,
		TAG_MORE, msg->ops_AttrList
	);

	if( !obj )
		return( 0 );

	data = INST_DATA( cl, obj );

	doset( data, obj, msg->ops_AttrList, NULL);

	return( (ULONG)obj );
}

DECDEST
{
	GETDATA;

	free( data->name );
	free( data->value );
	free( data->id );

	return( DOSUPER );
}

DECGET
{
	GETDATA;

	switch( (int)msg->opg_AttrID )
	{
		case MA_JS_Name:
		STOREATTR( MA_Layout_FormElement_Name, data->name );

		case MA_JS_ID:
		STOREATTR( MA_Layout_FormElement_DOMID, data->id );

		STOREATTR( MA_Layout_FormElement_Value, data->value );
		STOREATTR( MA_Layout_FormElement_Form, data->formobject );
	}

	return( DOSUPER );
}

DECSET
{
	GETDATA;
	int sendup = FALSE;

	doset( data, obj, msg->ops_AttrList, &sendup );

	if( sendup )
		return( DOSUPER );
	return( 0 );
}

BEGINMTABLE
DEFCONST
DEFDISPOSE
DEFGET
DEFSET
ENDMTABLE

int create_loformbuttonclass( void )
{
	if( !( lcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getlogroupmcc(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		lcc->mcc_Class->cl_ID = "loformbuttonClass";
#endif

	return( TRUE );
}

void delete_loformbuttonclass( void )
{
	if( lcc )
		MUI_DeleteCustomClass( lcc );
}

APTR getloformbuttonclass( void )
{
	return( lcc->mcc_Class );
}

APTR getloformbuttonmcc( void )
{
	return( lcc );
}
