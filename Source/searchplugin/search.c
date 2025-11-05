/*
		V3 Search Central plugin

		$Id: search.c,v 1.20 2003/07/06 16:51:36 olli Exp $
*/

#include <proto/exec.h>
#include <proto/dos.h>

#include <string.h>

#define BUILDPLUGIN
#include <libraries/v_plugin.h>

#include "rev.h"

/** "us" created by ShapeMe 1.6 ((7.12.96)) from "us.gif" **/ 

UBYTE usData[134]={0x47,0x49,0x46,0x38,0x39,0x61,0x12,0x00,0x0C,0x00,0xB3,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1C,0x1C,
		0xA0,0x8C,0x20,0x64,0x5C,0x5C,0x5C,0x8C,0x50,0x98,0xFC,0x30,0x34,0xFC,0x8C,0x8C,0xAC,
		0xAC,0xDC,0xF8,0xA4,0xA8,0xFC,0xB8,0xBC,0xDC,0xDC,0xDC,0x2C,0x00,0x00,0x00,0x00,0x12,
		0x00,0x0C,0x00,0x40,0x04,0x3B,0xF0,0xC9,0x49,0x2B,0x6D,0x38,0xEB,0xDD,0xD0,0x33,0x60,
		0x78,0x28,0x64,0x59,0x7A,0x61,0x98,0x2C,0x6C,0xDB,0x7A,0x4C,0x2C,0x3B,0x5C,0xE6,0x99,
		0x78,0xAE,0xA0,0xA9,0x31,0xEA,0xBB,0x4F,0x2F,0xE1,0x28,0x1A,0x8D,0xBC,0xD4,0xCA,0xE5,
		0xBA,0x01,0x75,0x1E,0xA6,0x94,0xE9,0x41,0x58,0xAF,0xD8,0xAC,0x35,0x02,0x00,0x3B};

/**** end of "us" ****/

/** "uk" created by ShapeMe 1.6 ((7.12.96)) from "uk.gif" **/ 

UBYTE ukData[162]={0x47,0x49,0x46,0x38,0x39,0x61,0x12,0x00,0x0C,0x00,0xB3,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x08,
		0x68,0xFC,0x0C,0x10,0x5C,0x5C,0x5C,0x64,0x60,0x9C,0x9C,0x68,0x90,0x8C,0x84,0xB0,0xF4,
		0x60,0x60,0xE0,0xA8,0xB4,0xEC,0xD0,0xD8,0xDC,0xDC,0xDC,0x2C,0x00,0x00,0x00,0x00,0x12,
		0x00,0x0C,0x00,0x40,0x04,0x57,0xF0,0xC9,0x49,0x2B,0x35,0xC6,0x35,0xA6,0xD6,0x39,0x4B,
		0xE3,0x70,0x18,0xF2,0x34,0xA8,0xE6,0x38,0x9F,0xAA,0xA1,0x66,0xC2,0x88,0x89,0xE1,0x1D,
		0xB5,0xCC,0x38,0x8B,0xF9,0xFD,0xC0,0xE0,0xC7,0xB4,0x51,0x60,0x6C,0x9F,0x1A,0x66,0x31,
		0x32,0xED,0x16,0xC7,0x9B,0xD2,0xA0,0x98,0x99,0x30,0x55,0xCD,0x6D,0x91,0x6D,0x94,0x1E,
		0xC2,0xF0,0x8F,0x98,0x52,0xB5,0x54,0xA8,0x86,0x89,0xC9,0x60,0xD4,0xA4,0x86,0x84,0x86,
		0x63,0x42,0xD8,0xEF,0xF8,0xBC,0x3D,0x02,0x00,0x3B};

/**** end of "uk" ****/

/** "fr" created by ShapeMe 1.6 ((7.12.96)) from "fr.gif" **/

UBYTE frData[113]={0x47,0x49,0x46,0x38,0x39,0x61,0x12,0x00,0x0C,0x00,0xF2,0x00,0x00,0x00,0x00,0x00,
	0x04,0x00,0x9C,0xFC,0x00,0x04,0x5C,0x5C,0x5C,0xAC,0xA8,0xDC,0xFC,0x9C,0x9C,0xDC,0xDC,
	0xDC,0xFC,0xFC,0xFC,0x21,0xF9,0x04,0x01,0x00,0x00,0x00,0x00,0x2C,0x00,0x00,0x00,0x00,
	0x12,0x00,0x0C,0x00,0x02,0x03,0x36,0x68,0xBA,0xDC,0x1C,0x10,0x92,0x43,0x4F,0x11,0x18,
	0x0F,0x13,0x65,0xB5,0x99,0xC6,0x75,0x53,0x75,0x85,0x5B,0x17,0x94,0xD4,0x99,0xA5,0xE4,
	0xE7,0x8A,0x2A,0x0B,0xA2,0x63,0x64,0xCF,0x02,0xAC,0xCB,0xA1,0x5E,0xCE,0x63,0x0A,0xFA,
	0x88,0x2D,0xA3,0x61,0xC0,0x6C,0x3A,0x9F,0xCC,0x04,0x00,0x3B};

/**** end of "fr" ****/

/** "de" created by ShapeMe 1.6 ((7.12.96)) from "de.gif" **/ 

UBYTE deData[108]={0x47,0x49,0x46,0x38,0x39,0x61,0x12,0x00,0x0C,0x00,0xF2,0x07,0x00,0x00,0x00,0x00,
	0x94,0x00,0x04,0xFC,0x00,0x04,0x5C,0x5C,0x5C,0xFC,0x48,0x04,0xFC,0xC4,0x04,0xDC,0xDC,
	0xDC,0xFF,0xFF,0xFF,0x21,0xF9,0x04,0x01,0x00,0x00,0x07,0x00,0x2C,0x00,0x00,0x00,0x00,
	0x12,0x00,0x0C,0x00,0x02,0x03,0x31,0x68,0xBA,0xDC,0x0C,0x30,0xCA,0x09,0x86,0xA1,0x38,
	0xDA,0x9C,0x6D,0xF8,0x60,0x28,0x06,0x96,0x60,0x9E,0x68,0x2A,0x94,0x6A,0x6B,0x5A,0x44,
	0x2C,0xCF,0x34,0x61,0x15,0x78,0xAE,0xEF,0xC5,0xCD,0xFF,0x38,0x1F,0x90,0x67,0x19,0x18,
	0x8F,0xC8,0xA4,0x31,0x01,0x00,0x3B};

/**** end of "de" ****/



// Move lib down the liblist (we're nice, aren't we?)
long __asm __saveds __UserLibInit( register __a6 struct Library *libbase )
{
		libbase->lib_Node.ln_Pri = -128;
		return( 0 );
}

struct TagItem tags[] = {

		VPLUG_Query_Version, VERSION,
		VPLUG_Query_Revision, REVISION,
		VPLUG_Query_Infostring, (ULONG)"Voyager Search Central plugin",
		VPLUG_Query_Copyright, (ULONG)"(C) 1997-2003 Oliver Wagner <owagner@vapor.com>, All Rights Reserved",

		VPLUG_Query_RegisterURLMethod, (ULONG)"SEARCH:",

		VPLUG_Query_HasURLMethodGetSize, TRUE,

		TAG_DONE
};

struct TagItem * __asm __saveds VPLUG_Query( void )
{
		return tags;
}

//      sprintf() replacement
void __stdargs sprintf( char *to, char *fmt, ... )
{
		static UWORD fmtfunc[] = { 0x16c0, 0x4e75 };
		RawDoFmt( fmt, &fmt + 1, (APTR)fmtfunc, to );
}

/*
"<td bgcolor=#e0e0e0><font size=-1><input type=submit name=s_m value='CDNOW'>&nbsp;by <select name=s_ac>"\
"<option value=a>Artist"\
"<option value=t>Title"\
"<option value=s>Song Title"\
"<option value=l>Record Label"\
"<option value=m>Movie Title"\
"<option value=r>Actor/Actress"\
"</select>"\
*/

#define TABLETOP \
" <TR BGCOLOR='Black'>\n"\
"  <TD>\n"\
"   <TABLE BORDER='0' CELLSPACING='0' CELLPADDING='2' WIDTH='100%%'>\n"\
"    <TR><TD><FONT COLOR='White'><TT>Voyager Search Central</TT></FONT></TD></TR>\n"\
"   </TABLE>\n"\
"  </TD>\n"\
" </TR>\n"

#define QUERYFORM \
"<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 3.2//EN'>\n"\
"<HTML>\n"\
"<HEAD>\n"\
"<!--  V³ Search Plugin HTML by Ben Preece (beej@vapor.com)  -->\n"\
"<TITLE>Voyager Search Central</TITLE>\n"\
"</HEAD>\n"\
"<BODY BGCOLOR='White' TEXT='Black'>\n"\
"<CENTER>\n"\
"<TABLE BORDER='0' CELLSPACING='0' CELLPADDING='1'>\n"\
"<FORM ACTION='search:' TARGET='_top'>\n"\
"%s <TR BGCOLOR='Black'>\n"\
"  <TD>\n"\
"   <TABLE BORDER='0' CELLSPACING='0' CELLPADDING='4' WIDTH='100%%'>\n"\
"    <TR>\n"\
"     <TD BGCOLOR='White' ALIGN='CENTER' COLSPAN='4'>\n"\
"<B>Enter keyword(s):</B>\n"\
"<INPUT TYPE='TEXT' NAME='k' SIZE='50' VALUE='%s'>\n"\
"<SELECT NAME='s_w'>\n"\
"<OPTION VALUE='sw_f'>Alltheweb\n"\
"<OPTION VALUE='sw_b'>Altavista\n"\
"<OPTION VALUE='sw_e'>Excite\n"\
"<OPTION VALUE='sw_a' SELECTED>Google\n"\
"<OPTION VALUE='sw_c'>Metacrawler\n"\
"<OPTION VALUE='sw_d'>Yahoo\n"\
"</SELECT>\n"\
"<INPUT TYPE='SUBMIT' NAME='s_m' VALUE=' Search '>\n"\
"     </TD>\n"\
"    </TR>\n"\
"    <TR>\n"\
"     <TD BGCOLOR='White' ALIGN='CENTER'><FONT SIZE='-1'>\n"\
"&nbsp;<A TARGET='_top' HREF='http://www.amazon.com/exec/obidos/redirect-home/vaporware'><IMG SRC='_us_img' WIDTH='18' HEIGHT='12' ALT='Amazon US' HSPACE='0' VSPACE='0' BORDER='0'></A>\n"\
"<SELECT NAME='s_a'>\n"\
"<OPTION VALUE='blended'>Everything\n"\
"<OPTION VALUE='books'>Books\n"\
"<OPTION VALUE='music'>Pop music\n"\
"<OPTION VALUE='classical-music'>Classical music\n"\
"<OPTION VALUE='video'>Videos/DVDs\n"\
"<OPTION VALUE='video:dvd'>DVDs\n"\
"<OPTION VALUE='video:vhs'>Videos\n"\
"</SELECT>\n"\
"@<INPUT TYPE='SUBMIT' NAME='s_m' VALUE=' Amazon.com '>\n"\
"     </FONT></TD>\n"\
"     <TD BGCOLOR='White' ALIGN='CENTER'><FONT SIZE='-1'>\n"\
"&nbsp;<A TARGET='_top' HREF='http://www.amazon.co.uk/exec/obidos/redirect-home/v3portal'><IMG SRC='_uk_img' WIDTH='18' HEIGHT='12' ALT='Amazon UK' HSPACE='0' VSPACE='0' BORDER='0'></A>\n"\
"<SELECT NAME='s_au'>\n"\
"<OPTION VALUE='blended'>Everything\n"\
"<OPTION VALUE='books'>Books\n"\
"<OPTION VALUE='music'>Pop music\n"\
"<OPTION VALUE='classical'>Classical music\n"\
"<OPTION VALUE='video'>Videos/DVDs\n"\
"<OPTION VALUE='video:dvd'>DVDs\n"\
"<OPTION VALUE='video:vhs'>Videos\n"\
"</SELECT>\n"\
"@<INPUT TYPE='SUBMIT' NAME='s_m' VALUE=' Amazon.co.uk '>\n"\
"     </FONT></TD>\n"\
"     <TD BGCOLOR='White' ALIGN='CENTER'><FONT SIZE='-1'>\n"\
"&nbsp;<A TARGET='_top' HREF='http://www.amazon.de/exec/obidos/redirect-home/vaporware05'><IMG SRC='_de_img' WIDTH='18' HEIGHT='12' ALT='Amazon DE' HSPACE='0' VSPACE='0' BORDER='0'></A>\n"\
"<SELECT NAME='s_ad'>\n"\
"<OPTION VALUE='blended'>Allem\n"\
"<OPTION VALUE='books'>Bücher\n"\
"<OPTION VALUE='us'>US-Bücher\n"\
"<OPTION VALUE='music'>Pop-Musik\n"\
"<OPTION VALUE='classical'>Klassische Musik\n"\
"<OPTION VALUE='video'>Video/DVDs\n"\
"<OPTION VALUE='video:dvd'>DVDs\n"\
"<OPTION VALUE='video:vhs'>Videos\n"\
"</SELECT>\n"\
"@<INPUT TYPE='SUBMIT' NAME='s_m' VALUE=' Amazon.de '>\n"\
"     </FONT></TD>\n"\
"     <TD BGCOLOR='White' ALIGN='CENTER'><FONT SIZE='-1'>\n"\
"&nbsp;<A TARGET='_top' HREF='http://www.amazon.fr/exec/obidos/redirect-home/thev3portal'><IMG SRC='_fr_img' WIDTH='18' HEIGHT='12' ALT='Amazon FR' HSPACE='0' VSPACE='0' BORDER='0'></A>\n"\
"<SELECT NAME='s_ad'>\n"\
"<OPTION VALUE='blended'>Tous les produits\n"\
"<OPTION VALUE='books-fr'>Livres en fran&ccedil;ais\n"\
"<OPTION VALUE='books-fr-intl-us'>Livres en anglais\n"\
"<OPTION VALUE='music-fr'>Pop, V.F., Jazz...\n"\
"<OPTION VALUE='classical-fr'>Classique\n"\
"<OPTION VALUE='vhs-fr'>Vid&eacute;o\n"\
"<OPTION VALUE='dvd-fr'>DVD\n"\
"</SELECT>\n"\
"@<INPUT TYPE='SUBMIT' NAME='s_m' VALUE=' Amazon.fr '>\n"\
"     </FONT></TD>\n"\
"     </TD>\n"\
"    </TR>\n"\
"   </TABLE>\n"\
"  </TD>\n"\
" </TR>\n"\
" <TR><TD COLSPAN='4' ALIGN='RIGHT' BGCOLOR='White'>"\
"<FONT SIZE='-2'><SCRIPT>if (top.window.frames[\"_frm_result\"]) document.write(\"<A HREF='javascript:top.location=top.window.frames[\\\"_frm_result\\\"].location'>Frame Breakout</A>\");</SCRIPT>\n"\
"&nbsp;&nbsp;Voyager Search Central " VERSIONSTRING " " REVDATE "</FONT></TD></TR>\n"\
"</TABLE>\n"\
"</FORM>\n"\
"<BR>\n"\
"<BR>\n"\
"</CENTER>\n"\
"</BODY>\n"\
"</HTML>\n"\

#define RESPONSE \
"<TITLE>Voyager Search Central</TITLE>\n"\
"<FRAMESET ROWS='90,*'>\n"\
"<FRAME SRC='search:%s' NORESIZE SCROLLING='NO'>\n"\
"<FRAME SRC='http://%s' NAME='_frm_result'>\n"\
"</FRAMESET>\n"

static void deescape( char *p )
{
		while( *p )
		{
				if( *p == '+' )
						*p = ' ';
				else if( *p == '%' && p[ 1 ] && p[ 2 ] )
				{
						char hex[ 4 ];
						long v;

						hex[ 0 ] = p[ 1 ];
						hex[ 1 ] = p[ 2 ];
						hex[ 2 ] = 0;

						stch_l( hex, &v );
						*p = v;
						strcpy( p + 1, p + 3 );
				}
				p++;
		}
}

APTR __asm __saveds VPLUG_ProcessURLMethod( register __a0 STRPTR url )
{
		char *res, *p;
		char temp[ 4096 ];

		res = AllocVec( 4096 - 4, 0 );
		strcpy( res, "<TITLE>Search Error</TITLE><BODY><H2>Internal error" );

		// ok, check the stuff
		p = strchr( url, ':' );
		if( p++ )
		{
			p = stpblk( p );
			if( !*p )
			{
				sprintf( res, QUERYFORM, TABLETOP, "" );
			}
			else if( !strcmp( p, "_us_img" ) )
			{
					FreeVec( res );
					return( usData );
			}
			else if( !strcmp( p, "_de_img" ) )
			{
					FreeVec( res );
					return( deData );
			}
			else if( !strcmp( p, "_uk_img" ) )
			{
					FreeVec( res );
					return( ukData );
			}
			else if( !strcmp( p, "_fr_img" ) )
			{
					FreeVec( res );
					return( frData );
			}
				else if( !strstr( p, "k=" ) )
				{
					stccpy( temp, p, sizeof( temp ) );
					deescape( temp );
					sprintf( res, QUERYFORM, "", temp );
				}
				else
				{
						char *keywords = NULL;
						char *s_m = NULL;
						char *s_w = NULL;
						char *s_a = NULL;
						char *s_au = NULL;
						char *s_ad = NULL;
						char respurl[ 512 ];

						// Split query string
						stccpy( temp, p, sizeof( temp ) );
						temp[ 0 ] = '&';

						respurl[ 0 ] = 0;

						for( p = strtok( temp, "&" ); p; p = strtok( NULL, "&" ) )
						{
								char *p2;
								p2 = strchr( p, '=' );
								if( !p2 )
										continue;
								*p2++ = 0;
								if( !strcmp( p, "k" ) )
										keywords = p2;
								else if( !strcmp( p, "s_m" ) )
										s_m = p2;
								else if( !strcmp( p, "s_w" ) )
										s_w = p2;
								else if( !strcmp( p, "s_a" ) )
										s_a = p2;
								else if( !strcmp( p, "s_ad" ) )
										s_ad = p2;
								else if( !strcmp( p, "s_au" ) )
										s_au = p2;
						}

						if( keywords && s_m )
						{
								if( !strcmp( s_m, "+Search+" ) )
								{
										sprintf( respurl, "service.bfast.com/bfast/click?siteid=27970546&bfpage=vertical&bfmid=27253343&q=%s", keywords );
										if( s_w )
										{
												if( !strcmp( s_w, "sw_b" ) )
														sprintf( respurl, "www.altavista.com/cgi-bin/search?pg=q&q=%s", keywords );
												else if( !strcmp( s_w, "sw_c" ) )
														sprintf( respurl, "search.metacrawler.com/crawler?general=%s", keywords );
												else if( !strcmp( s_w, "sw_d" ) )
														sprintf( respurl, "search.yahoo.com/bin/search?p=%s", keywords );
												else if( !strcmp( s_w, "sw_e" ) )
														sprintf( respurl, "search.excite.com/search.gw?search=%s", keywords );
												else if( !strcmp( s_w, "sw_f" ) )
														sprintf( respurl, "www.alltheweb.com/cgi-bin/search?query=%s", keywords );
										}
								}
								else if( !strcmp( s_m, "+Amazon.de+" ) )
								{
										sprintf( respurl, 
												"www.amazon.de/exec/obidos/external-search/?tag=vaporware05&keyword=%s&mode=%s", 
												keywords,
												s_ad
										);
								}
								else if( !strcmp( s_m, "+Amazon.fr+" ) )
								{
										sprintf( respurl, 
												"www.amazon.fr/exec/obidos/external-search/?tag=thev3portal&keyword=%s&mode=%s", 
												keywords,
												s_ad
										);
								}
								else if( !strcmp( s_m, "+Amazon.co.uk+" ) )
								{
										sprintf( respurl, 
												"www.amazon.co.uk/exec/obidos/external-search/?tag=v3portal&keyword=%s&mode=%s", 
												keywords,
												s_au
										);
								}
								else if( !strcmp( s_m, "+Amazon.com+" ) )
								{
										sprintf( respurl, 
												"www.amazon.com/exec/obidos/external-search/?tag=vaporware&keyword=%s&mode=%s", 
												keywords,
												s_a
										);
								}
						}

						if( respurl[ 0 ] )
						{
								deescape( keywords );
								sprintf( res, RESPONSE, keywords, respurl );
						}
				}
		}

		return( res );
}

STRPTR __asm __saveds VPLUG_GetURLData( register __a0 APTR urlhandle )
{
		return( urlhandle );
}

int __asm __saveds VPLUG_GetURLDataSize( register __a0 APTR urlhandle )
{
		if( urlhandle == (APTR)usData )
				return( sizeof( usData ) );
		else if( urlhandle == (APTR)deData )
				return( sizeof( deData ) );
		else if( urlhandle == (APTR)ukData )
				return( sizeof( ukData ) );
		else if( urlhandle == (APTR)frData )
				return( sizeof( frData ) );
		else
				return( -1 );
}

STRPTR __asm __saveds VPLUG_GetURLMIMEType( register __a0 APTR urlhandle )
{
		return( "text/html" );
}

void __asm __saveds VPLUG_FreeURLData( register __a0 APTR urlhandle )
{
		if( urlhandle != (APTR)usData &&
				urlhandle != (APTR)deData &&
				urlhandle != (APTR)ukData &&
				urlhandle != (APTR)frData
		)
		FreeVec( urlhandle );
}

void __asm __saveds VPLUG_Setup( void )
{
}
void __asm __saveds VPLUG_FinalSetup( void )
{
}
void __asm __saveds VPLUG_Cleanup( void )
{
}
void __asm __saveds VPLUG_Hook_Prefs( void )
{
}
void __asm __saveds VPLUG_GetClass( void )
{
}
void __asm __saveds VPLUG_GetInfo( void )
{
}
void __asm __saveds VPLUG_ProcessURLString( void )
{
}
