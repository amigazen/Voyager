/*
   $Id: about.c,v 1.36 2003/06/23 20:55:05 olli Exp $
*/

#include <proto/exec.h>
#include <exec/libraries.h>

#include "/includestuff/macros/compilers.h" /* XXX: hackish.. */

#ifdef __MORPHOS__
struct ExecBase *SysBase; /* lazy */
#define FAR
int lib_init( struct ExecBase *SBase )
{
	SysBase = SBase;
	return ( TRUE );
}

void lib_cleanup( void )
{
}

#define sprintf( a, b, ... ) NewRawDoFmt(b, 0, a, __VA_ARGS__ )
#endif

#ifdef __SASC
#define FAR
void __saveds __XCEXIT(void)
{
}

void __regargs __chkabort(void)
{
}

void __regargs _CXBRK(void)
{
}

long ASM SAVEDS __UserLibInit( register __a6 struct Library *libbase )
{
	libbase->lib_Node.ln_Pri = -128;
	return( 0 );
}
#endif

extern UBYTE FAR v3logoData[ 9910 ];

#ifdef __SASC
//  sprintf() replacement
UWORD fmtfunc[] = { 0x16c0, 0x4e75 };
void __stdargs sprintf( char *to, char *fmt, ... )
{
	RawDoFmt( fmt, &fmt + 1, (APTR)fmtfunc, to );
}
#endif

#ifdef __MORPHOS__
int lib_open( void )
{
	return ( TRUE );
}
#endif

// Raise the 4096 if text is longer
static char abouttxt[ 8192 ];

STRPTR ASM SAVEDS VABOUT_GetAboutPtr(
	__reg( a0, char *revid ),
	__reg( a1, char *owner ),
	__reg( a2, char *imgdec )
)
{
	struct Library *ssl = OpenLibrary( "PROGDIR:Plugins/voyager_ssl.vlib", 1 );

	sprintf( abouttxt,

"<HTML>\n"
"<HEAD>\n"
"<TITLE>About Voyager</TITLE>\n"
"</HEAD>\n"
"\n"
"<BODY BGCOLOR='White' TEXT='Black'>\n"
"\n"
"<CENTER>\n"
"\n"
"<A HREF='https://zapek.com/software/voyager/'>\n"
"<IMG SRC='v3logo' WIDTH=220 HEIGHT=194 ALT='Voyager' BORDER=0><BR>\n"
"</A>\n"
"\n"
"<FONT SIZE='+4'><B>Voyager 3.5</B></FONT><P>\n"
"<B>%s</B><BR>\n"
"<FONT SIZE='+1'><I>Beta</I></FONT>\n"
"<BR>\n"
"<HR COLOR='Black' SIZE=1 WIDTH=500>\n"
"<BR>\n"
"&copy; 1995-2003 Oliver Wagner and David Gerber<BR>\n"
"All Rights Reserved.<BR>\n"
"<BR>\n"
"<FONT SIZE='+1'><I>Voyager V3 is forever dedicated to the memory of Oliver Wagner.</I></FONT><BR>\n"
"<BR>\n"
"History and notes: <A HREF='https://zapek.com/software/voyager/'>https://zapek.com/software/voyager/</A><BR>\n"
"</FONT>\n"
"<BR>\n"
"<HR COLOR='Black' SIZE=1 WIDTH=500>\n"
"<BR>\n"
"\n"
"<B>%s</B>\n"
"\n"
"<BR>\n"
"<BR>\n"
"\n"
"<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=5 BGCOLOR='Black' WIDTH=500>\n"
" <TR>\n"
"  <TD>\n"
"   <FONT COLOR='White'>\n"
"    <TT>Copyrights</TT>\n"
"   </FONT>\n"
"  </TD>\n"
" </TR>\n"
"</TABLE>\n"
"\n"
"<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=1 BGCOLOR='Black' WIDTH=500>\n"
" <TR>\n"
"  <TD>\n"
"\n"
"   <TABLE BORDER=0 CELLSPACING=0 CELLPADDING=10 WIDTH='100%%'>\n"
"    <TR>\n"
"     <TD BGCOLOR='White' ALIGN=CENTER>\n"
"      <B>Voyager V3 uses <A HREF='http://www.sasg.com/'>MUI</A></B><BR>\n"
"      &copy; 1992-2003 Stefan Stuntz<BR>\n"
"      <FONT SIZE='-1'>Without MUI, Voyager would not exist</FONT>\n"
"     </TD>\n"
"     <TD BGCOLOR='White' ALIGN=CENTER>\n"
"      <B>Logo and Toolbar images</B><BR>\n"
"      by Jason 'Morden' Murray\n"
"     </TD>\n"
"    </TR>\n"
"    <TR>\n"
"     <TD BGCOLOR='White' ALIGN=CENTER>\n"
"      <B>Installation script</B><BR>\n"
"      by Robert 'RobR' Reiswig\n"
"     </TD>\n"
"     <TD BGCOLOR='White' ALIGN=CENTER>\n"
"      <B>File lister icons</B><BR>\n"
"      by Kenneth Jennings\n"
"     </TD>\n"
"    </TR>\n"
"    <TR>\n"
"     <TD BGCOLOR='White' ALIGN=CENTER>\n"
"      <B>Documentation</B><BR>\n"
"      by noone, ATM...\n"
"     </TD>\n"
"     <TD BGCOLOR='White' ALIGN=CENTER>\n"
"      <B>Original Toolbar images</B><BR>\n"
"      by Robert 'Devo' Bradman\n"
"     </TD>\n"
"    </TR>\n"
"    <TR>\n"
"     <TD BGCOLOR='White' ALIGN=CENTER>\n"
"      <B>SpeedBar and Contact Manager MCCs</B><BR>\n"
"      by Simone 'Wiz' Tellini\n"
"     </TD>\n"
"     <TD BGCOLOR='White' ALIGN=CENTER>\n"
"      <B>TearOff and CompactWindow MCCs</B><BR>\n"
"      by Szymon Ulatowski\n"
"     </TD>\n"
"    </TR>\n"
"   </TABLE>\n"
"\n"
"  </TD>\n"
" </TR>\n"
"</TABLE>\n"
"\n"
"<BR>\n"
"<BR>\n"
"\n"
"<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=10 WIDTH=500>\n"
" <TR>\n"
"  <TD ALIGN=RIGHT>\n"
"   <B>PNG / JPEG</B>\n"
"  </TD>\n"
"  <TD>\n"
"   <B>Image decoder module</B> includes code based partly on work by Guy Eric Schalnat, Sam Bushnell, Andreas Dilger, Dave Martindale, Paul Schmidt, Tim Wegner, and the Independent JPEG Group\n"
"  </TD>\n"
" </TR>\n"
" <TR>\n"
"  <TD ALIGN=RIGHT>\n"
"   <A HREF='https://github.com/amigazen/AmiTLS'>\n"
"    <B>AmiTLS</B>\n"
"   </A>\n"
"  </TD>\n"
"  <TD>\n"
"   <B>HTTPS</B> uses voyager_ssl.vlib with amitls.library (TLS 1.2). OpenSSL 0.9 is not used.\n"
"  </TD>\n"
" </TR>\n"
"</TABLE>\n"
"\n"
"<BR>\n"
"<BR>\n"
"\n"
"<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=5 BGCOLOR='Black' WIDTH=500>\n"
" <TR>\n"
"  <TD>\n"
"   <FONT COLOR='White'>\n"
"    <TT>Credits</TT>\n"
"   </FONT>\n"
"  </TD>\n"
" </TR>\n"
"</TABLE>\n"
"\n"
"<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=1 BGCOLOR='Black' WIDTH=500>\n"
" <TR>\n"
"  <TD>\n"
"\n"
"   <TABLE BORDER=0 CELLSPACING=0 CELLPADDING=10 WIDTH='100%%'>\n"
"    <TR>\n"
"     <TD BGCOLOR='White' ALIGN=CENTER>\n"
"      <B>Created and Programmed by</B><BR>\n"
"      Oliver Wagner<BR>\n"
"     </TD>\n"
"    </TR>\n"
"    <TR>\n"
"     <TD BGCOLOR='White' ALIGN=CENTER>\n"
"      <B>Assistant programmers</B><BR>\n"
"      David 'Zapek' Gerber<BR>\n"
"      Jon 'Sircus' Bright<BR>\n"
"     </TD>\n"
"    </TR>\n"
"    <TR>\n"
"     <TD BGCOLOR='White' ALIGN=CENTER>\n"
"      <B>Build and test of Voyager V3 3.5.2</B><BR>\n"
"      amigazen project<BR>\n"
"      <A HREF='https://www.amigazen.com/'>https://www.amigazen.com/</A><BR>\n"
"     </TD>\n"
"    </TR>\n"
"    <TR>\n"
"     <TD BGCOLOR='White' ALIGN=CENTER>\n"
"      <B>Special thanks to</B><BR>\n"
"      <FONT SIZE='-1'>\n"
"Matt Sealey,  \n"
"Simone 'Wiz' Tellini,  \n"
"David 'Spooky' Ceulemans,  \n"
"Marcin 'Carlos' Orlowski,  \n"
"Chris 'Active' Wiles,  \n"
"Matthias 'MAndree' Andree,  \n"
"Holger 'Holle' Rabbach,  \n"
"Ben 'Beej' Preece,  \n"
"Jamie 'Entity' van den Berge,  \n"
"Dr.Karl 'Red Button' Bellve,  \n"
"Eric 'EricGir' Giroux,  \n"
"Jason 'Morden' Murray,  \n"
"Niels 'Bogomil' Heuer,  \n"
"Jens 'JD' Droege,  \n"
"Kenneth 'Kenneth' Jennings,  \n"
"Christoph 'ChrisDi' Dietz,  \n"
"Adam 'AdamH' Hough,  \n"
"Pascal 'Syrus' Rullier,  \n"
"Mark 'Tecno' Wilson,  \n"
"Michael 'Cap' Brenegan,  \n"
"Lars 'Locutus' Svoldgaard,  \n"
"Sigbj&oslash;rn 'CISC' Skj&aelig;ret\n"
"      </FONT>\n"
"     </TD>\n"
"    </TR>\n"
"    <TR>\n"
"     <TD BGCOLOR='White' ALIGN=CENTER>\n"
"      <B>...and many many more</B>\n"
"     </TD>\n"
"    </TR>\n"
"   </TABLE>\n"
"\n"
"  </TD>\n"
" </TR>\n"
"</TABLE>\n"
"\n"
"<BR>\n"
"<BR>\n"
"\n"
"<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=5 BGCOLOR='Black' WIDTH=500>\n"
" <TR>\n"
"  <TD>\n"
"   <FONT COLOR='White'>\n"
"    <TT>Version information</TT>\n"
"   </FONT>\n"
"  </TD>\n"
" </TR>\n"
"</TABLE>\n"
"\n"
"<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=1 BGCOLOR='Black' WIDTH=500>\n"
" <TR>\n"
"  <TD>\n"
"\n"
"   <TABLE BORDER=0 CELLSPACING=0 CELLPADDING=10 WIDTH='100%%'>\n"
"    <TR>\n"
"     <TD BGCOLOR='White' ALIGN=CENTER>\n"
"      <B>Image&nbsp;decoder module</B><BR>\n"
"      <TT>%s</TT>\n"
"     </TD>\n"
"    </TR>\n"
"    <TR>\n"
"     <TD BGCOLOR='White' ALIGN=CENTER>\n"
"      <B>SSL&nbsp;module</B><BR>\n"
"      <TT>%s</TT>\n"
"     </TD>\n"
"    </TR>\n"
"   </TABLE>\n"
"\n"
"  </TD>\n"
" </TR>\n"
"</TABLE>\n"
"\n"
"<BR>\n"
"<BR>\n"
"\n"
"</CENTER>\n"
"\n"
"</BODY>\n"
"</HTML>\n"
		,
		revid,
		owner,
		imgdec,
		ssl ? ssl->lib_IdString : "(not installed)"
	);

	CloseLibrary( ssl );

	return( abouttxt );
}

APTR ASM SAVEDS VABOUT_GetVLogo(
	__reg( a0, int *size )
)
{
	*size = 0;
	return( NULL );
}

APTR ASM SAVEDS VABOUT_GetV3Logo(
	__reg( a0, int *size )
)
{
	*size = sizeof( v3logoData );
	return( v3logoData );
}

APTR ASM SAVEDS VABOUT_GetFlashLogo(
	__reg( a0, int *size )
)
{
	*size = 0;
	return( NULL );
}

APTR ASM SAVEDS VABOUT_GetPNGLogo(
	__reg( a0, int *size )
)
{
	*size = 0;
	return( NULL );
}

APTR ASM SAVEDS VABOUT_GetSSLLogo(
	__reg( a0, int *size )
)
{
	*size = 0;
	return( NULL );
}

STRPTR ASM SAVEDS VABOUT_GetAboutIbeta( void )
{
	return(

"<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 3.2//EN'>\n"
"<HTML>\n"
"<HEAD>\n"
"<!-- About IBETA -->\n"
"<TITLE>About Voyager 3 IBETA</TITLE>\n"
"</HEAD>\n"
"<BODY BGCOLOR=Black TEXT=White>\n"
"<CENTER>\n"
"<BR>\n"
"<H1><B>Voyager 3</B></H1><BR><BR><BR>\n"
"Created, Written and Directed by<BR>\n"
"<FONT SIZE='+2' COLOR='Red'>Oliver Wagner</FONT><BR><BR>\n"
"1st Assistant Director<BR>\n"
"<FONT SIZE='+2' COLOR='Red'>David Gerber</FONT><BR><BR>\n"
"Cinematographer<BR>\n"
"<FONT SIZE='+2' COLOR='Red'>Jason Murray</FONT><BR><BR>\n"
"Love Interest<BR>\n"
"<FONT SIZE='+2' COLOR='Red'>Claudi</FONT><BR><BR>\n"
"Cast <BR>\n"
"<FONT SIZE='+2' COLOR='Red'>\n"
"Eric Giroux<BR>\n"
"Christoph Dietz<BR>\n"
"Frank Mosch<BR>\n"
"Lars Svoldgaard<BR>\n"
"Michael Fitzgerald<BR>\n"
"David Ceulemans<BR>\n"
"Ben Preece<BR>\n"
"Niels Heuer<BR>\n"
"Marcin Orlowski<BR>\n"
"Matthias Andree<BR>\n"
"Jens Droege<BR>\n"
"Kenneth Jennings<BR>\n"
"Adam Hough<BR>\n"
"Eike M. Lang<BR>\n"
"Pascal Rullier<BR>\n"
"Mark Wilson<BR>\n"
"</FONT><BR>\n"
"Special Effects<BR>\n"
"<FONT SIZE='+2' COLOR='Red'>Simone Tellini</FONT><BR><BR>\n"
"Catering<BR>\n"
"<FONT SIZE='+2' COLOR='Red'>Robert Reiswig</FONT><BR><BR>\n"
"Red Buttons<BR>\n"
"<FONT SIZE='+2' COLOR='Red'>Karl Bellve</FONT><BR><BR>\n"
"LydiaVision&reg; Technology developed by<BR>\n"
"<FONT SIZE='+2' COLOR='Red'>Troels Walsted Hansen</FONT><BR><BR>\n"
"Security<BR>\n"
"<FONT SIZE='+2' COLOR='Red'>\n"
"Michael Brenegan<BR>\n"
"Ian Kumlien<BR>\n"
"Christian Birchinger<BR>\n"
"James Gurney<BR>\n"
"Holger Rabbach<BR>\n"
"</FONT><BR>\n"
"Chief Slacker<BR>\n"
"<FONT SIZE='+2' COLOR='Red'>Jerome Fisher</FONT><BR><BR>\n"
"Nagging Machine<BR>\n"
"<FONT SIZE='+2' COLOR='Red'>Chris Wiles</FONT><BR><BR>\n"
"Mr.Wagner's car furnished by<BR>\n"
"<FONT SIZE='+2' COLOR='Red'>Skoda Auto a.s.</FONT><BR><BR>\n"
"Mr.Wagner's toup&eacute;e polished by<BR>\n"
"<FONT SIZE='+2' COLOR='Red'>Hairpieces-R-Us</FONT><BR><BR>\n"
"Wardrobe<BR>\n"
"<FONT SIZE='+2' COLOR='Red'>Ikea</FONT><BR><BR>\n"
"Best Boy<BR>\n"
"<FONT SIZE='+2' COLOR='Red'>Jamie van den Berge</FONT><BR><BR>\n"
"IBETA would like to thank<BR>\n"
"<FONT SIZE='+2' COLOR='Red'>\n"
"id Software<BR>\n"
"The Hun<BR>\n"
"</FONT><BR><BR><BR>\n"
"NO LAMERS WERE HARMED DURING THE MAKING OF THIS PRODUCT<BR>\n"
"<FONT SIZE='-2'>Well okay, a few might have been</FONT>\n"
"</CENTER>\n"
"</BODY>\n"
"</HTML>\n"

 );

}
