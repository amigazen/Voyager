#pragma pack(2)
#pragma pack(2)
//
// Term, Terminal Class, Term_mcc_private.h
//
// Copyright © 1996-99 by Mathias Mischler & Oliver Wagner
// All Rights Reserved
//
// Changes:
// 16.4  (13.09.96) New tag TCA_DESTRBS (1021)
// 16.4  (13.09.96) MUIM_Notify on TCA_OUTLEN possible
// 16.6  (13.09.96) New tag TCA_VT100PENSPEC (1022)
// 16.6  (13.09.96) New tag TCA_VT100BGPENSPEC (1023)
// 16.7  (14.09.96) New tag TCA_CURSORSTYLE (1024)
// 16.7  (14.09.96) New tag TCA_DELASBS (1025)
// 16.8  (14.09.96) MUIA_Version, MUIA_Revision supported
// 16.10 (15.09.96) New tag TCA_SELECT (1026)
// 16.10 (15.09.96) New method TCM_SELECTTOCLIP (1027)
// 16.16 (23.09.96) New method TCM_GETARGS (1028)
// 16.16 (23.09.96) New method TCM_SETARGS (1029)
// 16.18 (28.09.96) New method TCM_PASTEFROMCLIP (102a)
// 16.19 (28.09.96) TCCFG_FONT (102b)
// 16.23 (05.11.96) TCCFG_COL0 (102c) - TCCFG7 (1033)
// 18.01 (25.11.96) New emulation TCV_EMULATION_ANSI16
// 19.01 (05.02.97) TCA_FIRSTINSTALL (1047)
// 19.02 (12.02.97) TCA_TERMTYPE (1048)
// 20.2  (29.11.98) TCM_SAVE
// 23.0  (02.04.99) 
//

#ifndef TERM_MCC_H
#define TERM_MCC_H

#ifndef LIBRARIES_MUI_H
#pragma pack()
#pragma pack()
#include "libraries/mui.h"
#pragma pack(2)
#pragma pack(2)
#endif

#define MUIC_Term "Term.mcc"
#define TermObject MUI_NewObject(MUIC_Term

#define MUISERIALNR_TERMC 3900
#define TAGBASE_TERMC 			(TAG_USER | ( MUISERIALNR_TERMC << 16))

// Methods

#define TCM_WRITE 					( TAGBASE_TERMC | 0x1002 )	// Terminal Write, give STRPTR and Lenght as parameters
#define TCM_OUTFLUSH    		( TAGBASE_TERMC | 0x101d )	// Flush Output Buffer (Set to 0)

#define TCM_SCROLLER				( TAGBASE_TERMC | 0x1009 )	// Scroller Changed (Link Object with TCA_SCROLLER)

#define TCM_RESET       		( TAGBASE_TERMC | 0x100b )	// Reset Terminal
#define TCM_INIT						( TAGBASE_TERMC | 0x100c )	// Initialize Terminal

#define TCM_SELECTTOCLIP		( TAGBASE_TERMC | 0x1027 )	// Copies selected characters, lines to Clipboard (see TCA_SELECT)
#define TCM_PASTEFROMCLIP	  ( TAGBASE_TERMC | 0x102a )	// Pastes from Clipboard
#define TCM_SAVEPLAIN				( TAGBASE_TERMC | 0x100d )	// Save terminal contents as plain text
#define TCM_SAVESTYLE				( TAGBASE_TERMC | 0x100e )	// Save terminal contents with styles

#define TCM_SETABSXY  		  ( TAGBASE_TERMC | 0x1016 )	// *Private* Set Window Size in Characters
#define TCM_SCROLLUP				( TAGBASE_TERMC | 0x1003 )  // *Private* Force Scrollup
#define TCM_SCROLLDOWN			( TAGBASE_TERMC | 0x1004 )	// *Private* Force Scrolldown
#define TCM_GOTOXY					( TAGBASE_TERMC | 0x1008 )	// *Private* Set Cursor to X/Y

#define TCM_GETARGS					( TAGBASE_TERMC | 0x1028 )	// *Private* Copy Argument String
#define TCM_SETARGS					( TAGBASE_TERMC | 0x1029 )	// *Private* Set Argument String

#define TCM_SAVEPLAINFH				( TAGBASE_TERMC | 0x1050 )	// Save terminal contents as plain text
#define TCM_SAVESTYLEFH				( TAGBASE_TERMC | 0x1051 )	// Save terminal contents with styles


// Application Return ID's

#define	TCID_OUT						( TAGBASE_TERMC | 0x101f ) 	// ID Outbuffer changed to App.

#define	TCID_NEWSIZE				( TAGBASE_TERMC | 0x101e ) 	// *Private* ID Newsize to App.

// Attributes

#define TCA_SCROLLER 				( TAGBASE_TERMC | 0x1000 )	// [i.s] Set Scroller Object (Link Changes with TCM_SCROLLER)

#define TCA_OUTPTR      		( TAGBASE_TERMC | 0x101b )  // [.g.] Pointer to Output Buffer
#define TCA_OUTLEN      		( TAGBASE_TERMC | 0x101c )  // [.g.] Lenght of Output Buffer

#define TCA_EMULATION 			( TAGBASE_TERMC | 0x1001 )  // [igs] tx Set Emulation Mode, see Values below

#define TCA_ECHO						( TAGBASE_TERMC | 0x100a )  // [igs] e  Settings, Turn on lokal Echo (Default off)
#define TCA_8BIT						( TAGBASE_TERMC | 0x1013 ) 	// [igs] 8  Settings, Activate 8-Bit Mode (Default 7 Bit)
#define TCA_CRASLF					( TAGBASE_TERMC | 0x1014 )	// [igs] c  Settings, Parse CR as LF (Default no)
#define TCA_SWAPDELBS   		( TAGBASE_TERMC | 0x1015 )  // [igs] s  Settings, Swap DEL and BS (Default no)
#define TCA_LFASCRLF    		( TAGBASE_TERMC | 0x1017 )  // [igs] l  Settings, Interpret LF as CR/LF combination (Default no)
#define TCA_WRAP        		( TAGBASE_TERMC | 0x1018 )  // [igs] w  Settings, Activete Character Wrap (Default off)
#define TCA_JUMPSCROLL			( TAGBASE_TERMC | 0x1019 )  // [igs] j  Settings, Use Jumpscroll (Default off)
#define TCA_DESTRBS					( TAGBASE_TERMC | 0x1021 )	// [igs] d  Settings, Use destructive backspace (Default off)
#define TCA_DELASBS					( TAGBASE_TERMC | 0x1025 )	// [igs] b  Settings, Delete is same as Backspace (Default off)

#define TCA_SELECT					( TAGBASE_TERMC | 0x1026 )	// [igs] Select characters and lines to Copy&Paste (Default False) (see TCM_SELECTTOCLIP)
#define TCA_CURSORSTYLE			( TAGBASE_TERMC | 0x1024 )  // [igs] Cursor style (see valid TCV_CURSORSTYLE_* values)
#define TCA_VT100PENSPEC		( TAGBASE_TERMC | 0x1022 )  // [igs] VT100/TTY pen-colour (Default text colour)
#define TCA_VT100BGPENSPEC	( TAGBASE_TERMC	| 0x1023 )  // [igs] VT100/TTY background pen colour (Default window colour)

#define TCA_COLOUR					( TAGBASE_TERMC | 0x1005 )  // [igs] *Private* Set Colour direct, better use ESC-Sequence ESC[<31+n>m
#define TCA_BGCOLOUR				( TAGBASE_TERMC | 0x1006 )  // [igs] *Private* Set Colour direct, better use ESC-Sequence ESC[<41+n>m
#define TCA_STYLE						( TAGBASE_TERMC | 0x1007 )  // [igs] *Private* Set Colour direct, better use ESC-Sequence ESC[<n>m
#define TCA_WIDTH						( TAGBASE_TERMC | 0x100f )  // [ig.] *Private* Width in Pixels, NEVER set
#define TCA_HEIGHT					( TAGBASE_TERMC | 0x1010 )  // [ig.] *Private* Height in Pixels, NEVER set
#define TCA_CURSORX     		( TAGBASE_TERMC | 0x1011 )  // [igs] *Private* Cursor Position X, better use ESC-Sequence ESC[<x>G
#define TCA_CURSORY     		( TAGBASE_TERMC | 0x1012 )  // [igs] *Private* Cursor Position Y, better use ESC-Sequence ESC[<y>;<x>H
#define	TCA_DEBUG						( TAGBASE_TERMC | 0x1020 ) 	// [igs] *Private* Settings, Debug Mode On
#define TCA_DOIAC       		( TAGBASE_TERMC | 0x101a )  // [igs] *Private* Parse IAC Commands

#define TCA_FIRSTINSTALL    ( TAGBASE_TERMC | 0x1047 )  // [.gs] *Private* Pointer to first install structure
#define TCA_TERMTYPE        ( TAGBASE_TERMC | 0x1048 )  // [igs] *Private* Pointer to string with terminal type

// Configs

#define TCCFG_FONT	 				( TAGBASE_TERMC | 0x102b )  // *Private* Config value for fixed font

#define TCCFG_COL0	 				( TAGBASE_TERMC | 0x102c )  // *Private* Config value for ANSI colour 0
#define TCCFG_COL1	 				( TAGBASE_TERMC | 0x102d )  // *Private* Config value for ANSI colour 1
#define TCCFG_COL2	 				( TAGBASE_TERMC | 0x102e )  // *Private* Config value for ANSI colour 2
#define TCCFG_COL3	 				( TAGBASE_TERMC | 0x102f )  // *Private* Config value for ANSI colour 3
#define TCCFG_COL4	 				( TAGBASE_TERMC | 0x1030 )  // *Private* Config value for ANSI colour 4
#define TCCFG_COL5	 				( TAGBASE_TERMC | 0x1031 )  // *Private* Config value for ANSI colour 5
#define TCCFG_COL6	 				( TAGBASE_TERMC | 0x1032 )  // *Private* Config value for ANSI colour 6
#define TCCFG_COL7	 				( TAGBASE_TERMC | 0x1033 )  // *Private* Config value for ANSI colour 7

#define TCCFG_VTCOL0				( TAGBASE_TERMC | 0x1034 )  // *Private* Config value for VT100 colour 0
#define TCCFG_VTCOL1				( TAGBASE_TERMC | 0x1035 )  // *Private* Config value for VT100 colour 1

#define TCCFG_ECOL0	 				( TAGBASE_TERMC | 0x1036 )  // *Private* Config value for ANSI colour 0
#define TCCFG_ECOL1	 				( TAGBASE_TERMC | 0x1037 )  // *Private* Config value for ANSI colour 1
#define TCCFG_ECOL2	 				( TAGBASE_TERMC | 0x1038 )  // *Private* Config value for ANSI colour 2
#define TCCFG_ECOL3	 				( TAGBASE_TERMC | 0x1039 )  // *Private* Config value for ANSI colour 3
#define TCCFG_ECOL4	 				( TAGBASE_TERMC | 0x103a )  // *Private* Config value for ANSI colour 4
#define TCCFG_ECOL5	 				( TAGBASE_TERMC | 0x103b )  // *Private* Config value for ANSI colour 5
#define TCCFG_ECOL6	 				( TAGBASE_TERMC | 0x103c )  // *Private* Config value for ANSI colour 6
#define TCCFG_ECOL7	 				( TAGBASE_TERMC | 0x103d )  // *Private* Config value for ANSI colour 7
#define TCCFG_ECOL8	 				( TAGBASE_TERMC | 0x103e )  // *Private* Config value for ANSI colour 8
#define TCCFG_ECOL9	 				( TAGBASE_TERMC | 0x103f )  // *Private* Config value for ANSI colour 9
#define TCCFG_ECOL10 				( TAGBASE_TERMC | 0x1040 )  // *Private* Config value for ANSI colour 10
#define TCCFG_ECOL11 				( TAGBASE_TERMC | 0x1041 )  // *Private* Config value for ANSI colour 11
#define TCCFG_ECOL12 				( TAGBASE_TERMC | 0x1042 )  // *Private* Config value for ANSI colour 12
#define TCCFG_ECOL13 				( TAGBASE_TERMC | 0x1043 )  // *Private* Config value for ANSI colour 13
#define TCCFG_ECOL14 				( TAGBASE_TERMC | 0x1044 )  // *Private* Config value for ANSI colour 14
#define TCCFG_ECOL15 				( TAGBASE_TERMC | 0x1045 )  // *Private* Config value for ANSI colour 15

#define TCCFG_MAXBUFFER				( TAGBASE_TERMC | 0x1050 )  // *Private* Config value for maximum buffer size

// Values

#define TCV_EMULATION_ANSI		0											// ANSI X3.64 1979 Emulation (see TCA_EMULATION)
#define TCV_EMULATION_VT100 	1											// DEC VT100 Emulation (see TCA_EMULATION)
#define TCV_EMULATION_TTY			2											// Direct Text Output (see TCA_EMULATION)
#define TCV_EMULATION_ANSI16  3											// ANSI X3.64 1979 Emulation 16 colours (see TCA_EMULATION)

#define TCV_CURSORSTYLE_INVERSE			0								// Use inversid Cursor (see TCA_CURSORSTYLE)
#define TCV_CURSORSTYLE_UNDERLINED	1								// Use underlined Cursor (see TCA_CURSORSTYLE)

#define TCARGSTRINGLEN 16														// *Private* To alloc memory for args string copy

// Structs *Private*

struct  TCP_XY					{ ULONG MethodID; ULONG x; ULONG y; };
struct  TCP_WRITE       { ULONG MethodID; STRPTR str; ULONG len; };
struct  TCP_SAVE				{ ULONG MethodID; STRPTR filename; };
struct	TCP_ARGS				{ ULONG MethodID;	STRPTR args; };
struct  TCP_SAVEFH				{ ULONG MethodID; BPTR fh; };


#endif /* TERM_MCC_H */

#pragma pack()
#pragma pack()
