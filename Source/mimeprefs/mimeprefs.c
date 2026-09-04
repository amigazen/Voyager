#include "mimeprefs.h"
#include "rev.h"
#define CATCOMP_ARRAY
#include "mimeprefs_cat.h"

#define NUMCATSTRING (sizeof(CatCompArray) / sizeof(CatCompArray[0]))
char *__stringtable[ sizeof(CatCompArray) / sizeof(CatCompArray[0]) ];

char version[] = { "$VER: MIMEPrefs " VERTAG };
char copyright[] = { "MIMEPrefs " VERTAG " (C) 1997-2000 Oliver Wagner, All Rights Reserved" };

enum {
	ID_dummy = 1,

	MENU_OPEN,
	MENU_SAVEAS,
	MENU_ABOUT,
	MENU_DEFAULTS,
	MENU_LASTSAVED,
	MENU_RESTORE,
	MENU_IMPORT_V,
	MENU_CREATEICONS,
	MENU_MUI,
	MENU_TEST,

	ID_MAKE_INSERT_ACTIVE,
	ID_CHECK_CLASS,
	ID_SAVE,
	ID_USE,
	ID_TEST,
	ID_TESTVIEW,
};


// SAS/C Startup vars
long __stack = 16000;
long __oslibversion = 37;

//	sprintf() replacement (signature matches SAS/C stdlib.h)
UWORD fmtfunc[] = { 0x16c0, 0x4e75 };
int sprintf( char *to, const char *fmt, ... )
{
	RawDoFmt( fmt, &fmt + 1, (APTR)fmtfunc, to );
	return( 0 );
}

char * __vat_appid = "MimePrefs " VERTAG;
ULONG __vat_requirements = 0;

//
//  Open MUIMaster
//
struct Library *MUIMasterBase;
CONSTRUCTOR_P(openmuimaster,3500)
{
	struct EasyStruct eas;

	FOREVER
	{
		MUIMasterBase = (void *)OpenLibrary( MUIMASTER_NAME, 18 );

		if( MUIMasterBase )
			return( 0 );

		eas.es_StructSize = sizeof( eas );
		eas.es_Flags = 0;
		eas.es_Title = copyright;
		eas.es_TextFormat = "MimePrefs requires at least V18\nof \"" MUIMASTER_NAME "\"\nPlease install MUI 3.7 or higher!";
		eas.es_GadgetFormat = "Retry|Cancel";
		if( !EasyRequest( NULL, &eas, NULL ) )
			return( 1 );
   }
}

DESTRUCTOR_P(closemuimaster,3500)
{
	CloseLibrary( MUIMasterBase );
}

//
//	Construct LocaleInfo
//

struct Catalog *catalog;

CONSTRUCTOR_P(openlocale,10000)
{
	int c;

	for( c = 0; c < NUMCATSTRING; c++ )
		__stringtable[ c ] = CatCompArray[ c ].cca_Str;

	LocaleBase = (struct LocaleBase *)OpenLibrary( "locale.library", 0 );
	if( !LocaleBase )
		return( NULL );

	catalog = OpenCatalog(
		NULL, "MimePrefs.catalog",
		OC_BuiltInLanguage, "english",
		TAG_DONE
	);

	if( catalog )
	{
		for( c = 0; c < NUMCATSTRING; c++ )
			__stringtable[ c ] = GetCatalogStr( catalog, c, __stringtable[ c ] );
	}
	return( 0 );
}
DESTRUCTOR_P(closelocale,10000)
{
	if( catalog )
		CloseCatalog( catalog );
	CloseLibrary( (struct Library *)LocaleBase );
}

#undef set
#define set(o,a,v) myset(o,a,(ULONG)v)
static void myset( APTR obj, ULONG attr, ULONG v)
{
	SetAttrs( obj, attr, v, TAG_DONE );
}

#undef nnset
#define nnset(o,a,v) mynnset(o,a,(ULONG)v)
static void mynnset( APTR obj, ULONG attr, ULONG v)
{
	SetAttrs( obj, MUIA_NoNotify, TRUE, attr, v, TAG_DONE );
}

static void __stdargs multidis( int state, ... )
{
	APTR *op = (APTR*)(&state + 1);

	while( *op )
	{
		SetAttrs( *op, MUIA_Disabled, state, TAG_DONE );
		op++;
	}
}

//
//  Startup parsing...
//

extern struct WBStartup * _WBenchMsg;
struct RDArgs *rda;
struct myargs {
	char *from;
	ULONG edit;
	ULONG use;
	ULONG save;
	char *pubscreen;
	ULONG createicons;
	ULONG oldstylelists;
	char *appname;
} myargs;
char myfullpath[ 256 ];
char startup_cfgfile[ 256 ] = { "ENV:MIME.Prefs" };
struct DiskObject *diskobj;

CONSTRUCTOR_P(loaddiskobj,20000)
{
	char progname[ 128 ];

	if( !_WBenchMsg )
	{
		// Shell-Startup
		rda = ReadArgs( "From,Edit/S,Use/S,Save/S,PubScreen/K,CreateIcons/S,UseOldStyleLists=UOSL/S,AppName/K", (LONG*)&myargs, NULL );
		if( !rda )
		{
			PrintFault( IoErr(), "MimePrefs" );
			return( 20 );
		}

		if( myargs.from )
		{
			stccpy( startup_cfgfile, myargs.from, sizeof( startup_cfgfile ) );
		}

		NameFromLock( GetProgramDir(), myfullpath, sizeof( myfullpath ) );
		GetProgramName( progname, sizeof( progname ) );
		AddPart( myfullpath, progname, sizeof( myfullpath ) );
		diskobj = GetDiskObject( startup_cfgfile );
		if( !diskobj )
			diskobj = GetDiskObjectNew( myfullpath );
	}
	else
	{
		// WBStartup
		struct WBArg *wbarg;
		char fullpath[ 256 ];
		int num = _WBenchMsg->sm_NumArgs > 1 ? 1 : 0;
		char *tt = NULL;

		wbarg = _WBenchMsg->sm_ArgList;

		NameFromLock( wbarg->wa_Lock, myfullpath, 256 );
		AddPart( myfullpath, wbarg->wa_Name, 256 );

		// load program icon
		if( num )
			wbarg++;
		NameFromLock( wbarg->wa_Lock, fullpath, 256 );
		AddPart( fullpath, wbarg->wa_Name, 256 );
		diskobj = GetDiskObjectNew( fullpath );
		if( num )
			strcpy( startup_cfgfile, fullpath );

		tt = FindToolType( diskobj->do_ToolTypes, "ACTION" );
		if( tt )
		{
			if( MatchToolValue( tt, "USE" ))
				myargs.use = TRUE;
			if( MatchToolValue( tt, "SAVE" ))
				myargs.save = TRUE;
		}

		tt = FindToolType( diskobj->do_ToolTypes, "SAVE" );
		if( tt )
			myargs.save = TRUE;

		tt = FindToolType( diskobj->do_ToolTypes, "USE" );
		if( tt )
			myargs.use = TRUE;

		tt = FindToolType( diskobj->do_ToolTypes, "PUBSCREEN" );
		if( tt )
			myargs.pubscreen = tt;

		tt = FindToolType( diskobj->do_ToolTypes, "CREATEICONS" );
		if( tt && !MatchToolValue( tt, "NO" ) )
			myargs.createicons = TRUE;

		tt = FindToolType( diskobj->do_ToolTypes, "USEOLDSTYLELISTS" );
		if( tt && !MatchToolValue( tt, "NO" ) )
			myargs.oldstylelists = TRUE;
	}

	return( 0 );
}
DESTRUCTOR_P(freediskobj,20000)
{
	if( diskobj )
		FreeDiskObject( diskobj );
}

static char *class_l = "List.mui";
static char *class_lv = "Listview.mui";
static int using_nlist;

#undef MUIC_List
#define MUIC_List class_l
#undef MUIC_Listview
#define MUIC_Listview class_lv

CONSTRUCTOR_P(probenlist,20001)
{
	APTR o1, o2;

	if( myargs.oldstylelists )
		return( 0 );

	o1 = NListObject, End;
	o2 = NListviewObject, End;

	if( o1 && o2 )
	{
		int v;

		get( o1, MUIA_Version, &v );

		if( v >= 19 )
		{
			class_l = MUIC_NList;
			class_lv = MUIC_NListview;
			using_nlist = TRUE;
		}
	}

	if( o1 )
		MUI_DisposeObject( o1 );
	if( o2 )
		MUI_DisposeObject( o2 );

	return( 0 );
}

APTR app, win, menu;
APTR bt_save, bt_use, bt_cancel;
APTR lv_mime, str_mime_type, str_mime_ext, str_mime_app, rad_mime_act, bt_mime_del, cyc_mime_type, pop_mime_app;
APTR str_mime_store, chk_mime_store_class;
APTR grp_act;
APTR chk_view_internal, chk_view_stream;
APTR bt_add, bt_del, bt_copy, bt_testview;
APTR grp_view, grp_edit1, grp_edit2;
APTR win_test;
APTR str_testfile, bt_test, txt_test;

struct FileRequester *prefsfr;

static APTR but( char *str )
{
	APTR o = MUI_MakeObject( MUIO_Button, str );
	if( o )
		SetAttrs( o, MUIA_CycleChain, 1, TAG_DONE );
	return( o );
}

//
// MIME handling settings
//

STRPTR mimetype_opts[] = { "application/", "text/", "image/", "audio/", "video/", "message/", "multipart/", NULL };

static int __callback mime_disp_func( _reg( a1 ) struct mimeinfo *mi, _reg( a2 ) STRPTR *array )
{
	if( !mi )
	{
		*array++ = GS( MIME_LV1 ) + ( using_nlist ? 2 : 0 );
		*array++ = GS( MIME_LV2 ) + ( using_nlist ? 2 : 0 );
		*array++ = GS( MIME_LV3 ) + ( using_nlist ? 2 : 0 );
		*array++ = GS( MIME_LV4 ) + ( using_nlist ? 2 : 0 );
		*array++ = GS( MIME_LV6 ) + ( using_nlist ? 2 : 0 );
		*array = GS( MIME_LV5 ) + ( using_nlist ? 2 : 0 );
	}
	else
	{
		static char typebuff[ 256 ];
		static char flagbuff[ 4 ];

		strcpy( typebuff, mimetype_opts[ mi->basetype ] );
		strcat( typebuff, mi->type );

		*array++ = typebuff;
		*array++ = mi->ext;
		*array++ = GSI( MSG_MIME_LV_ACT1 + mi->act );

		if( mi->use_classdir )
			*array++ = GS( MIME_LV_CLASS );
		else if( !mi->dir[ 0 ] )
			*array++ = GS( MIME_LV_ASK );
		else
			*array++ = mi->dir;

		flagbuff[ 0 ] = mi->use_internal ? 'I' : ' ';
		flagbuff[ 1 ] = mi->use_stream ? 'S' : ' ';
		flagbuff[ 2 ] = 0;
		*array++ = flagbuff;

		*array = mi->app;
	}
	return( 0 );
}

static APTR __callback mime_const_func( _reg( a1 ) struct mimeinfo *mi, _reg( a2 ) APTR pool )
{
	struct mimeinfo *nmi = AllocPooled( pool, sizeof( *nmi ) );
	int newone = FALSE;

	//Printf( "const %lx\n", mi );

	if( nmi )
	{
		if( mi != (struct mimeinfo*)-2 )
		{
			if( mi == (struct mimeinfo*)-1 )
			{
				struct mimeinfo *mi2;
				DoMethod( lv_mime, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &mi2 );
				mi = mi2;
				newone = TRUE;
			}

			if( mi )
				*nmi = *mi;
		}
		else
		{
			memset( nmi, 0, sizeof( *nmi ) );
			newone = TRUE;
		}
	}

	if( newone )
		DoMethod( app, MUIM_Application_ReturnID, ID_MAKE_INSERT_ACTIVE );

	return( nmi );
}

static int __callback mime_compare_func( _reg( a1 ) struct mimeinfo *m1, _reg( a2 ) struct mimeinfo *m2 )
{
	if( m1->basetype == m2->basetype )
		return( stricmp( m1->type, m2->type ) );
	else
		return( ( m1->basetype > m2->basetype ) ? 1 : -1 );
}

static void __callback mime_dest_func( _reg( a1 ) struct mimeinfo *mi, _reg( a2 ) APTR pool )
{
	FreePooled( pool, mi, sizeof( *mi ) );
}

static void __callback mime_act_func( _reg( a1 ) LONG *args )
{
	DoMethod( grp_edit1, MUIM_Group_InitChange );
	DoMethod( grp_edit2, MUIM_Group_InitChange );

	DoMethod( app, MUIM_MultiSet, MUIA_Disabled, ( *args ) == MUIV_List_Active_Off,
		grp_edit1, grp_edit2, bt_del, bt_copy, bt_testview, NULL
	);
	if( *args != MUIV_List_Active_Off )
	{
		struct mimeinfo *mi;

		DoMethod( lv_mime, MUIM_List_GetEntry, *args, &mi );

		nnset( str_mime_type, MUIA_String_Contents, mi->type );
		nnset( str_mime_ext, MUIA_String_Contents, mi->ext );
		nnset( str_mime_app, MUIA_String_Contents, mi->app );
		nnset( str_mime_store, MUIA_String_Contents, mi->dir );
		nnset( rad_mime_act, MUIA_Radio_Active, mi->act );
		nnset( cyc_mime_type, MUIA_Cycle_Active, mi->basetype );
		nnset( chk_mime_store_class, MUIA_Selected, mi->use_classdir );
		nnset( chk_view_internal, MUIA_Selected, mi->use_internal );
		nnset( chk_view_stream, MUIA_Selected, mi->use_stream );

		//DoMethod( app, MUIM_Application_PushMethod, grp_view, 3, MUIM_Set, MUIA_Disabled, mi->act <= 1 );
		set( str_mime_store, MUIA_Disabled, mi->use_classdir );
		set( str_mime_ext, MUIA_Disabled, !strcmp( mi->type, "*" ) );
		set( grp_view, MUIA_Disabled, mi->act <= 1 );
		set( bt_testview, MUIA_Disabled, mi->act <= 1 );
	}

	DoMethod( grp_edit1, MUIM_Group_ExitChange );
	DoMethod( grp_edit2, MUIM_Group_ExitChange );
}

static void copystr( APTR str, char *to )
{
	char *p;
	get( str, MUIA_String_Contents, &p );
	strcpy( to, p );
}

static ULONG getv( APTR obj, ULONG attr )
{
	ULONG v;

	get( obj, attr, &v );
	return( v );
}

static void __callback mime_copy_func( void )
{
	struct mimeinfo *mi;

	DoMethod( lv_mime, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &mi );
	if( mi )
	{
		copystr( str_mime_type, mi->type );
		copystr( str_mime_ext, mi->ext );
		copystr( str_mime_app, mi->app );
		copystr( str_mime_store, mi->dir );
		mi->act = getv( rad_mime_act, MUIA_Radio_Active );
		mi->basetype = getv( cyc_mime_type, MUIA_Cycle_Active );
		mi->use_stream = getv( chk_view_stream, MUIA_Selected );
		mi->use_internal = getv( chk_view_internal, MUIA_Selected );
		mi->use_classdir = getv( chk_mime_store_class, MUIA_Selected );
		DoMethod( lv_mime, MUIM_List_Redraw, MUIV_List_Redraw_Active );
		set( str_mime_ext, MUIA_Disabled, !strcmp( mi->type, "*" ) );
	}
}

DEFHOOK( mime_const );
DEFHOOK( mime_dest );
DEFHOOK( mime_disp );
DEFHOOK( mime_act );
DEFHOOK( mime_copy );
DEFHOOK( mime_compare );

#define MENU(x) (STRPTR)MSG_MENU_##x

struct NewMenu newmenus[] = {
NM_TITLE, MENU(PROJECT),		  		0,   0, 0, 0,
 NM_ITEM, MENU(OPEN),			   		0,	 0, 0, (APTR)MENU_OPEN,
 NM_ITEM, MENU(SAVEAS),			   		0,	 0, 0, (APTR)MENU_SAVEAS,
 NM_ITEM, NM_BARLABEL,			    	0,   0, 0, NULL,
 NM_ITEM, MENU(ABOUT),			   		0,	 0, 0, (APTR)MENU_ABOUT,
 NM_ITEM, NM_BARLABEL,			    	0,   0, 0, NULL,
 NM_ITEM, MENU(QUIT),			   		0,	 0, 0, (APTR)MUIV_Application_ReturnID_Quit,

NM_TITLE, MENU(EDIT),		 	 		0,   0, 0, 0,
 NM_ITEM, MENU(DEFAULTS),			 	0,	 0, 0, (APTR)MENU_DEFAULTS,
 NM_ITEM, MENU(LASTSAVED),			 	0,	 0, 0, (APTR)MENU_LASTSAVED,
 NM_ITEM, MENU(RESTORE),			 	0,	 0, 0, (APTR)MENU_RESTORE,
 NM_ITEM, NM_BARLABEL,			    	0,   0, 0, NULL,
 NM_ITEM, MENU(IMPORT_V),			 	0,	 0, 0, (APTR)MENU_IMPORT_V,
 NM_ITEM, NM_BARLABEL,			    	0,   0, 0, NULL,
 NM_ITEM, MENU(TEST),				 	0,	 0, 0, (APTR)MENU_TEST,

NM_TITLE, MENU(SETTINGS),		  		0,   0, 0, 0,
 NM_ITEM, MENU(CREATEICONS), 			0, CHECKIT | MENUTOGGLE, 0, (APTR)MENU_CREATEICONS,
 NM_ITEM, MENU(MUI),			 		0,	 0, 0, (APTR)MENU_MUI,

NM_END };

static void initnewmenus( void )
{
	struct NewMenu *nmp = newmenus;

	while( nmp->nm_Type )
	{
		if( nmp->nm_Label != NM_BARLABEL )
			nmp->nm_Label = GSI( (ULONG)nmp->nm_Label );
		nmp++;
	}
}

static void __callback appmsg_func( register __a1 struct AppMessage **amp )
{
	struct AppMessage *am = *amp;
	char filename[ 256 ];

	if( !am->am_NumArgs )
		return;

	NameFromLock( am->am_ArgList[ 0 ].wa_Lock, filename, sizeof( filename ) );
	AddPart( filename, am->am_ArgList[ 0 ].wa_Name, sizeof( filename ) );
	
	set( str_testfile, MUIA_String_Contents, filename );
}

DEFHOOK( appmsg );

static APTR shlabel1( char *l, char *sh )
{
	APTR o = MUI_MakeObject( MUIO_Label, l, MUIO_Label_SingleFrame | MUIO_Label_DontCopy );
	set( o, MUIA_ShortHelp, sh );
	return( o );
}

static APTR shlabel2( char *l, char *sh )
{
	APTR o = MUI_MakeObject( MUIO_Label, l, MUIO_Label_DoubleFrame | MUIO_Label_DontCopy );
	set( o, MUIA_ShortHelp, sh );
	return( o );
}

static void mch( APTR obj, ULONG attr )
{
	DoMethod( obj, MUIM_Notify, attr, MUIV_EveryTime,
		lv_mime, 3, MUIM_CallHook, &mime_copy_hook, 0
	);
}

static int buildapp( void )
{
	static STRPTR classlist[] = { "Textinput.mcp", "NListviews.mcp", NULL };
	static STRPTR mime_opts[ 5 ];
	int c;

	for( c = 0; c < 4; c++ )
		mime_opts[ c ] = GSI( MSG_MIME_ACTS_1 + c );

	initnewmenus();

	app = ApplicationObject,
		MUIA_Application_Title, "MIMEPrefs",
		MUIA_Application_Version, version,
		MUIA_Application_Copyright, copyright,
		MUIA_Application_Author, "Oliver Wagner",
		MUIA_Application_UsedClasses, classlist,
		MUIA_Application_Description, GS( APP_DESC ),
		MUIA_Application_Base, "MIMEPREFS",
		MUIA_Application_HelpFile, GS( APP_GUIDENAME ),
		MUIA_Application_Menustrip, menu = MUI_MakeObject( MUIO_MenustripNM, newmenus, MUIO_MenustripNM_CommandKeyCheck ),
		//MUIA_Application_Commands, rexxcmds,
		MUIA_Application_DiskObject, diskobj,
		MUIA_Application_SingleTask, TRUE,

		SubWindow, win_test = WindowObject,
			MUIA_Window_ID   , MAKE_ID('T','E','S','T'),
			MUIA_Window_Title, GS( WINTEST_TITLE ),
			MUIA_Window_ScreenTitle, copyright,
			MUIA_Window_AppWindow, TRUE,
			myargs.pubscreen ? MUIA_Window_PublicScreen : TAG_IGNORE, myargs.pubscreen,
			MUIA_Window_RootObject, VGroup,

				Child, HGroup,

					Child, shlabel2( GS( WINTEST_FILE ), GS( SH_WINTEST_FILE ) ),

					Child, str_testfile = PopaslObject,
						MUIA_ShortHelp, GS( SH_WINTEST_FILE ),
						MUIA_Popasl_Type, ASL_FileRequest,
						MUIA_Popstring_String, TextinputObject, StringFrame, MUIA_String_MaxLen, 256, MUIA_CycleChain, 1, End,
						MUIA_Popstring_Button, PopButton( MUII_PopFile ),
						ASLFR_TitleText, GS( WINTEST_FILE ),
					End,

					Child, bt_test = but( GS( WINTEST_DO ) ),

				End,

				Child, txt_test = ListviewObject,
					MUIA_Listview_List, FloattextObject, ReadListFrame,
					End,
				End,
			End,
		End,

		SubWindow, win = WindowObject,
			MUIA_Window_ID   , MAKE_ID('M','A','I','N'),
			MUIA_Window_Title, GS( WIN_TITLE ),
			MUIA_Window_ScreenTitle, copyright,
			//MUIA_Window_AppWindow, TRUE,
			myargs.pubscreen ? MUIA_Window_PublicScreen : TAG_IGNORE, myargs.pubscreen,
			//MUIA_Window_SizeRight, TRUE,
			MUIA_Window_RootObject, VGroup,

				Child, lv_mime = ListviewObject, MUIA_CycleChain, 1,
					MUIA_Listview_DragType, MUIV_Listview_DragType_Immediate,
					MUIA_Listview_List, ListObject, InputListFrame, 
						MUIA_List_DragSortable, TRUE,
						MUIA_List_ConstructHook, &mime_const_hook,
						MUIA_List_DestructHook, &mime_dest_hook,
						MUIA_List_DisplayHook, &mime_disp_hook,
						MUIA_List_CompareHook, &mime_compare_hook,
						MUIA_List_Format, "BAR,BAR,BAR,BAR,BAR,",
						MUIA_List_Title, TRUE,
					End,
				End,

				Child, grp_edit1 = ColGroup( 2 ),

					Child, shlabel2( GS( MIME_E_TYPE ), GS( SH_MIME_TYPE ) ),
					Child, HGroup, MUIA_Group_Spacing, 0,
						Child, cyc_mime_type = Cycle( mimetype_opts ),
						Child, str_mime_type = TextinputObject, StringFrame, MUIA_CycleChain, TRUE, MUIA_String_MaxLen, 256, MUIA_ShortHelp, GS( SH_MIME_TYPE ), End,
					End,

					Child, shlabel2( GS( MIME_E_EXT ), GS( SH_MIME_EXT ) ),
					Child, str_mime_ext = TextinputObject, StringFrame, MUIA_CycleChain, TRUE, MUIA_String_MaxLen, 256, MUIA_ShortHelp, GS( SH_MIME_EXT ), MUIA_String_Reject, "\"", End,

					Child, Label2( GS( MIME_E_STORE ) ),
					Child, HGroup,
						Child, str_mime_store = PopaslObject,
							MUIA_Popasl_Type, ASL_FileRequest,
							MUIA_Popstring_String, TextinputObject, StringFrame, MUIA_CycleChain, TRUE, MUIA_String_MaxLen, 256, MUIA_ShortHelp, GS( SH_MIME_STORE ), End,
							MUIA_Popstring_Button, PopButton( MUII_PopDrawer ),
							ASLFR_DrawersOnly, TRUE,
							ASLFR_TitleText, GS( MIME_E_STORE ),
						End,
						Child, shlabel1( GS( MIME_E_UCD ), GS( SH_MIME_STORE ) ),
						Child, chk_mime_store_class = CheckMark( FALSE ),
					End,
				End,

				Child, grp_edit2 = HGroup,

					Child, rad_mime_act = RadioObject,
						MUIA_Radio_Entries, mime_opts,
						MUIA_Weight, 0,
						MUIA_CycleChain, 1,
						MUIA_ShortHelp, GS( SH_MIME_ACT ),
					End,

					Child, MUI_MakeObject( MUIO_VBar, 1 ),

					Child, grp_view = VGroup,

						Child, MUI_MakeObject( MUIO_BarTitle, GS( VIEWER ) ),

						Child, str_mime_app = PopaslObject,
							MUIA_ShortHelp, GS( SH_MIME_APP ),
							MUIA_Popasl_Type, ASL_FileRequest,
							MUIA_Popstring_String, TextinputObject, StringFrame, MUIA_String_MaxLen, 256, MUIA_CycleChain, 1, End,
							MUIA_Popstring_Button, PopButton( MUII_PopFile ),
							ASLFR_TitleText, GS( MIME_E_APP ),
						End,
						Child, TextObject, MUIA_Text_Contents, GS( VIEWINFO ), MUIA_Font, MUIV_Font_Tiny, MUIA_Text_SetVMax, FALSE, End,
						Child, HGroup,
							Child, shlabel1( GS( VIEW_USE_INTERNAL ), GS( SH_MIME_INTERNAL ) ),
							Child, chk_view_internal = CheckMark( TRUE ),
							Child, HSpace( 0 ),
							Child, shlabel1( GS( VIEW_USE_STREAM ), GS( SH_MIME_STREAM ) ),
							Child, chk_view_stream = CheckMark( FALSE ),
						End,
					End,
				End,

				Child, HGroup,
					Child, bt_add = but( GS( BT_ADD ) ),
					Child, bt_del = but( GS( BT_DEL ) ),
					Child, bt_copy = but( GS( BT_COPY ) ),
					Child, bt_testview = but( GS( BT_TESTVIEW ) ),
				End,

				Child, MUI_MakeObject( MUIO_HBar, 1 ),

				Child, HGroup,
					Child, bt_save = but( GS( BT_SAVE ) ),
					Child, bt_use = but( GS( BT_USE ) ),
					Child, bt_cancel = but( GS( BT_CANCEL ) ),
				End,

			End,
		End,
	End;

	if( !app )
	{
		int rc = MUI_Error();

		if( !rc || rc == 6 )
			return( 5 );

		MUI_Request( NULL, NULL, 0, copyright, GS( CANCEL ), GS( APP_FAILED ), rc );
		return( 10 );
	}

	SetAttrs( cyc_mime_type,
		MUIA_Weight, 0,
		MUIA_CycleChain, 1,
		MUIA_ShortHelp, GS( SH_MIME_CLASS ),
		TAG_DONE
	);

	SetAttrs( chk_mime_store_class,
		MUIA_CycleChain, 1,
		MUIA_ShortHelp, GS( SH_MIME_STORE ),
		TAG_DONE
	);

	SetAttrs( chk_view_internal,
		MUIA_CycleChain, 1,
		MUIA_ShortHelp, GS( SH_MIME_INTERNAL ),
		TAG_DONE
	);

	SetAttrs( chk_view_stream,
		MUIA_CycleChain, 1,
		MUIA_ShortHelp, GS( SH_MIME_STREAM ),
		TAG_DONE
	);

	SetAttrs( bt_test, 
		MUIA_Weight, 0,
		MUIA_ShortHelp, GS( SH_WINTEST_FILE ),
		TAG_DONE
	);

	// Notifications
	DoMethod( win, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit
	);

	DoMethod( bt_cancel, MUIM_Notify, MUIA_Pressed, FALSE,
		app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit
	);
	DoMethod( bt_save, MUIM_Notify, MUIA_Pressed, FALSE,
		app, 2, MUIM_Application_ReturnID, ID_SAVE
	);
	DoMethod( bt_use, MUIM_Notify, MUIA_Pressed, FALSE,
		app, 2, MUIM_Application_ReturnID, ID_USE
	);

	for( c = 0; c < 4; c++ )
	{
		DoMethod( rad_mime_act, MUIM_Notify, MUIA_Radio_Active, c,
			grp_view, 3, MUIM_Set, MUIA_Disabled, c <= 1
		);
		DoMethod( rad_mime_act, MUIM_Notify, MUIA_Radio_Active, c,
			bt_testview, 3, MUIM_Set, MUIA_Disabled, c <= 1
		);
	}

	DoMethod( lv_mime, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
		lv_mime, 3, MUIM_CallHook, &mime_act_hook, MUIV_TriggerValue
	);

	DoMethod( bt_add, MUIM_Notify, MUIA_Pressed, FALSE,
		lv_mime, 3, MUIM_List_InsertSingle, -2, MUIV_List_Insert_Bottom
	);
	DoMethod( bt_del, MUIM_Notify, MUIA_Pressed, FALSE,
		lv_mime, 2, MUIM_List_Remove, MUIV_List_Remove_Active
	);
	DoMethod( bt_copy, MUIM_Notify, MUIA_Pressed, FALSE,
		lv_mime, 3, MUIM_List_InsertSingle, -1, MUIV_List_Insert_Bottom
	);

	DoMethod( bt_test, MUIM_Notify, MUIA_Pressed, FALSE,
		app, 2, MUIM_Application_ReturnID, ID_TEST
	);
	DoMethod( bt_testview, MUIM_Notify, MUIA_Pressed, FALSE,
		app, 2, MUIM_Application_ReturnID, ID_TESTVIEW
	);

	DoMethod( str_testfile, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
		app, 2, MUIM_Application_ReturnID, ID_TEST
	);

	DoMethod( chk_mime_store_class, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
		str_mime_store, 3, MUIM_Set, MUIA_Disabled, MUIV_TriggerValue
	);

	DoMethod( chk_mime_store_class, MUIM_Notify, MUIA_Selected, TRUE,
		app, 2, MUIM_Application_ReturnID, ID_CHECK_CLASS
	);

	DoMethod( win_test, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		win_test, 3, MUIM_Set, MUIA_Window_Open, FALSE
	);

	// copy triggers
	mch( cyc_mime_type, MUIA_Cycle_Active );
	mch( rad_mime_act, MUIA_Radio_Active );
	mch( str_mime_store, MUIA_String_Contents );
	mch( str_mime_type, MUIA_String_Contents );
	mch( str_mime_app, MUIA_String_Contents );
	mch( str_mime_ext, MUIA_String_Contents );
	mch( chk_view_internal, MUIA_Selected );
	mch( chk_view_stream, MUIA_Selected );
	mch( chk_mime_store_class, MUIA_Selected );

	// set disable states
	{
		LONG v = MUIV_List_Active_Off;
		mime_act_func( &v );
	}

	DoMethod( win_test, MUIM_Notify, MUIA_AppMessage, MUIV_EveryTime,
		win_test, 3, MUIM_CallHook, &appmsg_hook, MUIV_TriggerValue
	);

	prefsfr = MUI_AllocAslRequestTags( ASL_FileRequest,
		ASLFR_InitialDrawer, "SYS:Prefs/Presets",
		ASLFR_RejectIcons, TRUE,
		TAG_DONE
	);

	DoMethod( app, MUIM_Application_SetMenuCheck, MENU_CREATEICONS, myargs.createicons );

	return( 0 );
}

CONSTRUCTOR_P(buildapp,21000)
{
	return( buildapp() );
}
DESTRUCTOR_P(buildapp,21000)
{
	if( prefsfr )
		MUI_FreeAslRequest( prefsfr );
	if( app )
		MUI_DisposeObject( app );
}

static void nofilereq( char *name )
{
	char fb[ 128 ];

	Fault( IoErr(), NULL, fb, sizeof( fb ) );

	MUI_Request( app, win, 0, GS( ERROR ), GS( CANCEL ), GS( NOFILE ), name, fb, IoErr() );
}

static int loadprefs( char *from )
{
	char buffer[ 512 ];
	BPTR f;
	char *p, *p2;
	struct mimeinfo mi;
	int c;

	f = Open( from, MODE_OLDFILE );
	if( !f )
	{
		nofilereq( from );
		return( FALSE );
	}

	set( lv_mime, MUIA_List_Quiet, TRUE );
	DoMethod( lv_mime, MUIM_List_Clear );

	for(;;)
	{
		if( !FGets( f, buffer, sizeof( buffer ) ) )
			break;

		p = stpbrk( buffer, ";\r\n" );
		if( p )
			*p = 0;

		memset( &mi, 0, sizeof( mi ) );

		p = stpbrk( buffer, "," );
		if( !p )
			continue;
		*p++ = 0;

		for( c = 0; mimetype_opts[ c ]; c++ )
		{
			if( !strncmp( buffer, mimetype_opts[ c ], strlen( mimetype_opts[ c ] ) ) )
				break;
		}
		if( !mimetype_opts[ c ] )
			continue;

		mi.basetype = c;
		p2 = strchr( buffer, '/' );
		if( !p2 )
			continue;
		strcpy( mi.type, p2 + 1 );

		p2 = p;
		p = stpbrk( p, "," );
		if( !p )
			continue;
		*p++ = 0;
		strcpy( mi.ext, p2 );

		p2 = p;
		p = stpbrk( p, "," );
		if( !p )
			continue;
		*p++ = 0;
		strcpy( mi.dir, p2 );

		p2 = p;
		p = stpbrk( p, "," );
		if( !p )
			continue;
		*p++ = 0;
		strcpy( mi.app, p2 );		

		p2 = p;
		p = stpbrk( p, "," );
		if( !p )
			continue;
		*p++ = 0;
		mi.act = atoi( p2 );

		p2 = p;
		p = stpbrk( p, "," );
		if( !p )
			continue;
		*p++ = 0;
		mi.use_classdir = atoi( p2 );

		p2 = p;
		p = stpbrk( p, "," );
		if( !p )
			continue;
		*p++ = 0;
		mi.use_stream = atoi( p2 );

		mi.use_internal = atoi( p );

		DoMethod( lv_mime, MUIM_List_InsertSingle, &mi, MUIV_List_Insert_Bottom );
	}
	Close( f );

	set( lv_mime, MUIA_List_Quiet, FALSE );
}

static int saveprefs( char *to, int noicons )
{
	BPTR f;
	struct mimeinfo *mi;
	int c;

	f = Open( to, MODE_NEWFILE );
	if( !f )
	{
		nofilereq( to );
		return( FALSE );
	}

	FPrintf( f, ";\n; MIME Preferences\n\;\n; generated with MimePrefs " VERTAG " \n;\n" );

	for( c = 0; ; c++ )
	{
		DoMethod( lv_mime, MUIM_List_GetEntry, c, &mi );
		if( !mi )
			break;

		FPrintf( f, "%s%s,%s,%s,%s,%ld,%ld,%ld,%ld\n",
			mimetype_opts[ mi->basetype ], mi->type,
			mi->ext,
			mi->dir,
			mi->app,
			mi->act,
			mi->use_classdir ? 1 : 0,
			mi->use_stream ? 1 : 0,
			mi->use_internal ? 1 : 0
		);
	}

	FPrintf( f, ";\n; EOF\n;\n" );
	Close( f );

	if( !noicons && DoMethod( app, MUIM_Application_GetMenuCheck, MENU_CREATEICONS ) )
	{
		struct DiskObject *dob;

		dob = GetDiskObjectNew( "ENV:SYS/def_pref" );
		if( dob )
		{
			dob->do_DefaultTool = "SYS:Prefs/MIME";
			dob->do_StackSize = 16384;
			dob->do_Type = WBPROJECT;
			PutDiskObject( to, dob );
			FreeDiskObject( dob );
		}
	}
}

static void loadprefsfh( void )
{
	struct Window *w;

	get( win, MUIA_Window_Window, &w );

	if( MUI_AslRequestTags( prefsfr,
		ASLFR_TitleText, GS( FR_LOAD ),
		ASLFR_DoSaveMode, FALSE,
		ASLFR_Window, w,
		ASLFR_SleepWindow, TRUE
	))
	{
		char path[ 256 ];

		strcpy( path, prefsfr->fr_Drawer );
		AddPart( path, prefsfr->fr_File, sizeof( path ) );
		loadprefs( path );
	}
}

static void saveprefsfh( void )
{
	struct Window *w;

	get( win, MUIA_Window_Window, &w );

	if( MUI_AslRequestTags( prefsfr,
		ASLFR_TitleText, GS( FR_SAVE ),
		ASLFR_DoSaveMode, TRUE,
		ASLFR_Window, w,
		ASLFR_SleepWindow, TRUE
	))
	{
		char path[ 256 ];

		strcpy( path, prefsfr->fr_Drawer );
		AddPart( path, prefsfr->fr_File, sizeof( path ) );
		saveprefs( path, FALSE );
	}
}


static void importprefs( int mode )
{
	struct FileRequester *fr;
	struct Window *w;
	char *deffile[] = { "Voyager.prefs" };
	ULONG deftext[] = { MSG_IMPORT_V };

	get( win, MUIA_Window_Window, &w );

	fr = MUI_AllocAslRequestTags( ASL_FileRequest,
		ASLFR_Window, w,
		ASLFR_SleepWindow, TRUE,
		ASLFR_InitialFile, deffile[ mode ],
		ASLFR_TitleText, GSI( deftext[ mode ] ),
		TAG_DONE
	);
	if( fr )
	{
		if( MUI_AslRequestTags( fr, TAG_DONE ) )
		{
			char path[ 256 ];
			int rc;

			strcpy( path, fr->fr_Drawer );
			AddPart( path, fr->fr_File, sizeof( path ) );

			switch( mode )
			{
				case 0:
					rc = importvprefs( path );
					break;
			}

			if( rc )
				nofilereq( path );
		}
		MUI_FreeAslRequest( fr );
	}
}

static void makedefaults( void )
{
	struct mimeinfo mi;
	int c;
	struct mdef {
		UBYTE basetype;
		UBYTE act;
		char *type;
		char *ext;
	} mdefs[] = {
		0, 1, "*", "",
		0, 1, "octet-stream", "exe info",
		0, 1, "x-zip", "zip",
		0, 1, "x-tar", "tar tgz",
		0, 1, "x-lha", "lha lzh",
		0, 1, "x-lzx", "lzx",

		1, 3, "*", "",
		1, 3, "plain", "txt doc readme text",
		1, 3, "html", "htm html shtml",
		1, 3, "x-amigaguide", "guide",
		1, 3, "rtf", "rtf",

		2, 3, "*", "",
		2, 3, "x-ilbm", "ilbm iff brush",
		2, 3, "gif", "gif",
		2, 3, "jpeg", "jpg jfif jpeg",
		2, 3, "png", "png",
		2, 3, "bmp", "bmp",
		2, 3, "xbm", "xbm",

		3, 3, "*", "",
		3, 3, "x-8svx", "8svx",
		3, 3, "wav", "wav voc",
		3, 3, "au", "au",
		3, 3, "mpeg", "mp3",

		4, 3, "*", "",
		4, 3, "x-anim", "anim",		
		4, 3, "avi", "avi",
		4, 3, "mpeg", "mpeg",

		0, 0, NULL, NULL
	};

	memset( &mi, 0, sizeof( mi ) );

	strcpy( mi.dir, "RAM:" );

	set( lv_mime, MUIA_List_Quiet, TRUE );
	DoMethod( lv_mime, MUIM_List_Clear );

	for( c = 0; mdefs[ c ].type; c++ )
	{
		strcpy( mi.ext, mdefs[ c ].ext );
		strcpy( mi.type, mdefs[ c ].type );
		mi.basetype = mdefs[ c ].basetype;
		mi.act = mdefs[ c ].act;
		mi.use_classdir = strcmp( mi.type, "*" );
		if( mi.act >= 2 )
		{
			strcpy( mi.app, "SYS:Utilities/Multiview %f PUBSCREEN %p" );
			mi.use_internal = TRUE;
		}
		else
		{
			mi.app[ 0 ] = 0;
			mi.use_internal = FALSE;
		}
		DoMethod( lv_mime, MUIM_List_InsertSingle, &mi, MUIV_List_Insert_Bottom );
	}

	set( lv_mime, MUIA_List_Quiet, FALSE );
}

static void runtest( struct mimeinfo *mi, char *filename, char *buffexec )
{
	char *p, *p2;

	p2 = mi->app;
	p = buffexec;

	while( *p2 )
	{
		if( *p2 == '%' )
		{
			p2++;
			if( *p2 == 'f' )
			{
				sprintf( p, "\"%s\"", filename );
				p = strchr( p, 0 );
				p2++;
			}
			else if( *p2 == 'p' )
			{
				sprintf( p, "\"%s\"", VAT_GetAppScreenName( app ) );
				p = strchr( p, 0 );

				p2++;
			}
			else
				*p++ = *p2++;
		}
		else
		{
			*p++ = *p2++;
		}
	}
	*p = 0;

	if( mi->use_stream )
	{
		SystemTags( buffexec,
			SYS_Asynch, TRUE,
			SYS_Input, Open( filename, MODE_OLDFILE ),
			SYS_Output, Open( "NIL:", MODE_NEWFILE ),
			TAG_DONE
		);
	}
	else
	{
		SystemTags( buffexec,
			SYS_Asynch, TRUE,
			SYS_Input, Open( "NIL:", MODE_NEWFILE ),
			SYS_Output, Open( "NIL:", MODE_NEWFILE ),
			TAG_DONE
		);
	}
}

static void dotest( void )
{
	char *filename, *p, *p2;
	char buffer[ 1024 ];
	char buffexec[ 512 ];
	struct mimeinfo *mi;
	int c;

	get( str_testfile, MUIA_String_Contents, &filename );

	sprintf( buffer, GS( WINTEST_T_HEADER ), filename );

	p = strrchr( filename, '.' );
	if( !p )
	{
		strcat( buffer, GS( WINTEST_T_UNKNOWN ) );
		set( txt_test, MUIA_Floattext_Text, buffer );
		return;
	}
	p++;

	for( c = 0; ; c++ )
	{
		DoMethod( lv_mime, MUIM_List_GetEntry, c, &mi );
		if( !mi )
		{
			strcat( buffer, GS( WINTEST_T_UNKNOWN ) );
			set( txt_test, MUIA_Floattext_Text, buffer );
			return;
		}

		strcpy( buffexec, mi->ext );

		for( p2 = strtok( buffexec, " ." ); p2; p2 = strtok( NULL, " ." ) )
		{
			if( !stricmp( p2, p ) )
				break;
		}

		if( p2 )
			break;
	}

	// found
	sprintf( strchr( buffer, 0 ), GS( WINTEST_T_INFO ),
		mimetype_opts[ mi->basetype ], mi->type,
		GSI( MSG_MIME_ACTS_1 + mi->act )
	);

	if( mi->act >= 2 )
	{
		runtest( mi, filename, buffexec );
		sprintf( strchr( buffer, 0 ), GS( WINTEST_T_RUN ), buffexec );
	}

	set( txt_test, MUIA_Floattext_Text, buffer );
}

static void checkclass( void )
{
	struct mimeinfo *mi, *mi2;
	int c;

	DoMethod( lv_mime, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &mi );
	if( !mi || !strcmp( mi->type, "*" ) )
	{
		set( chk_mime_store_class, MUIA_Selected, FALSE );
		return;
	}

	for( c = 0; ; c++ )
	{
		DoMethod( lv_mime, MUIM_List_GetEntry, c, &mi2 );
		if( !mi2 )
			break;

		if( mi2->basetype == mi->basetype )
			if( !strcmp( mi2->type, "*" ) )
				return;
	}

	if( MUI_Request( app, win, 0, NULL, GS( MIME_NOCLASS_ASK ), GS( MIME_NOCLASS ), mimetype_opts[ mi->basetype ] ) )
	{
		struct mimeinfo minew;

		minew = *mi;
		minew.ext[ 0 ] = 0;
		strcpy( minew.type, "*" );
		minew.use_classdir = FALSE;

		DoMethod( lv_mime, MUIM_List_InsertSingle, &minew, MUIV_List_Insert_Active );
	}
	else
		set( chk_mime_store_class, MUIA_Selected, FALSE );
}

static void dotestview( void )
{
	struct FileRequester *fr;
	struct mimeinfo *mi;
	char buffexec[ 512 ];
	struct Window *w;

	DoMethod( lv_mime, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &mi );
	if( !mi )
		return;

	get( win, MUIA_Window_Window, &w );

	fr = MUI_AllocAslRequestTags( ASL_FileRequest,
		ASLFR_TitleText, GS( TESTVIEW_FILETITLE ),
		ASLFR_Window, w,
		ASLFR_InitialDrawer, mi->dir,
		ASLFR_SleepWindow, TRUE,
		TAG_DONE
	);

	if( fr )
	{
		if( MUI_AslRequestTags( fr, TAG_DONE ) )
		{
			char name[ 256 ];

			strcpy( name, fr->fr_Drawer );
			AddPart( name, fr->fr_File, sizeof( name ) );

			runtest( mi, name, buffexec );
		}
		MUI_FreeAslRequest( fr );
	}
}

static void doloop( void )
{
	LONG id;
	ULONG msig, sig = 0;
	int Done = FALSE;

	while( !Done )
	{
		msig = sig;

		id = DoMethod( app, MUIM_Application_NewInput, &msig );

		switch( id )
		{
			case MENU_OPEN:
				loadprefsfh();
				break;

			case MENU_SAVEAS:
				saveprefsfh();
				break;

			case MENU_LASTSAVED:
				loadprefs( "ENVARC:MIME.prefs" );
				break;

			case MENU_RESTORE:
				loadprefs( "ENV:MIME.prefs" );
				break;

			case MENU_DEFAULTS:
				makedefaults();
				break;

			case ID_SAVE:
				saveprefs( "ENVARC:MIME.prefs", TRUE );
				// fallthrough

			case ID_USE:
				saveprefs( "ENV:MIME.prefs", TRUE );
				// fallthrough

			case MUIV_Application_ReturnID_Quit:
				Done = TRUE;
				break;

			case MENU_ABOUT:
				MUI_Request( app, win, 0, GS( MENU_ABOUT ) + 2, GS( OK ), "\033c\033bMimePrefs " VERTAG "\033n\n\n%s\n\n  1997-2000 Oliver Wagner\n<owagner@vapor.com>\nAll Rights Reserved", GS( APP_DESC ) );
				break;

			case MENU_MUI:
				DoMethod( app, MUIM_Application_OpenConfigWindow, 0 );
				break;

			case ID_MAKE_INSERT_ACTIVE:
				set( lv_mime, MUIA_List_Active, getv( lv_mime, MUIA_List_InsertPosition ) );
				break;

			case MENU_TEST:
				set( win_test, MUIA_Window_Open, TRUE );
				break;

			case MENU_IMPORT_V:
				importprefs( 0 );
				break;

			case ID_TEST:
				dotest();
				break;

			case ID_CHECK_CLASS:
				checkclass();
				break;

			case ID_TESTVIEW:
				dotestview();
				break;
		}

		if( Done )
			break;

		if( msig )
		{
			sig = Wait( msig | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F );

			if( sig & SIGBREAKF_CTRL_C )
				Done = TRUE;

			if( sig & SIGBREAKF_CTRL_F )
				set( app, MUIA_Application_Iconified, FALSE );
		}
	}
}

int main( int argc, char **argv )
{
	makedefaults();
	loadprefs( startup_cfgfile );

	if( myargs.use )
		DoMethod( app, MUIM_Application_ReturnID, ID_USE );
	else if( myargs.save )
		DoMethod( app, MUIM_Application_ReturnID, ID_SAVE );
	else	
		set( win, MUIA_Window_Open, TRUE );

	doloop();

	return( 0 );
}

// Update stuff

struct Library *VUPBase;
APTR vuphandle;

CONSTRUCTOR_P(chkupdate,29999)
{
	VUPBase = OpenLibrary( "vapor_update.library", 1 );
	if( VUPBase )
	{
		vuphandle = VUP_BeginCheckUpdate( 4, VERHEXID, "MimePrefs " VERTAG );
	}
	return( 0 );
}
DESTRUCTOR_P(chkupdate,29999)
{
	if( VUPBase )
	{
		VUP_Quit( vuphandle );
		CloseLibrary( VUPBase );
	}
}
