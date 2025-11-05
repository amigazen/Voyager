// $Id: swf.h,v 1.2 2004/03/03 03:10:56 zapek Exp $
#ifndef _SWF_H_
#define _SWF_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <assert.h>
#include <limits.h>

#ifdef DUMP
#include "bitstream.h"
#endif

#include "flash.h"

extern int debug;

// Global Types
typedef unsigned long U32, *P_U32, **PP_U32;
typedef signed long S32, *P_S32, **PP_S32;
typedef unsigned short U16, *P_U16, **PP_U16;
typedef signed short S16, *P_S16, **PP_S16;
typedef unsigned char U8, *P_U8, **PP_U8;
typedef signed char S8, *P_S8, **PP_S8;
typedef signed long SFIXED, *P_SFIXED;
typedef signed long SCOORD, *P_SCOORD;
typedef unsigned long BOOL32;
typedef float FLOAT;

#define ZOOM(v,f) ((v)/(f))

#include "matrix.h"
#include "cxform.h"
#include "rect.h"

#ifndef MBX
#include <sys/time.h>
#endif /* !MBX */
/*
 * Everyone is against me. The timer.device author... the ADE guys...
 */
#undef tv_sec
#undef tv_usec

#define ST struct unixtimeval t1,t2;

#ifdef MBX
#undef START  // used in include/hardware/igs5050hal.h as well - and this is fetched from modules/inspiration/pointer.h
#undef STOP
#ifdef __DCC__
//#pragma pack(1)		// Do packing !
//#warn "Packing enabled for all structures"
#endif
#ifdef __GNUC__
// #warn "Packing must be enabled for all structures"
//** Is there some way to tell GNU not to pad ??
#endif
#endif

#define START gettimeofday(&t1,0)
#define STOP(msg) gettimeofday(&t2,0); printf("%s Delta = %d ms\n", msg, (t2.tv_sec-t1.tv_sec)*1000+(t2.tv_usec-t1.tv_usec)/1000); fflush(stdout);

// Start Sound Flags
enum {
	soundHasInPoint		= 0x01,
	soundHasOutPoint	= 0x02,
	soundHasLoops		= 0x04,
	soundHasEnvelope	= 0x08

	// the upper 4 bits are reserved for synchronization flags
};

// Flags for Sound Format
enum SounfFlags {
	soundIsStereo		= 0x01,
	soundIs16bit		= 0x02,
	soundIsADPCMCompressed	= 0x10,
	soundIsMP3Compressed    = 0x20
};

// Flags for defining Button States
enum ButtonState {
	stateHitTest = 0x08,
	stateDown    = 0x04,
	stateOver    = 0x02,
	stateUp      = 0x01
};

// Actions
enum Action {
		// Internal actions
		ActionRefresh       = 0x00,
		ActionPlaySound     = 0x01,
		// Flash 1 and 2
		ActionGotoFrame	    = 0x81, // frame num (WORD)
		ActionGetURL        = 0x83, // url (STR), window (STR)
		ActionNextFrame	    = 0x04,
		ActionPrevFrame	    = 0x05,
		ActionPlay          = 0x06,
		ActionStop          = 0x07,
		ActionToggleQuality	= 0x08,
		ActionStopSounds    = 0x09,
		ActionWaitForFrame  = 0x8a, // frame needed (WORD), actions to skip (BYTE)
		
		// Flash 3
		ActionSetTarget	    = 0x8b, // name (STR)
		ActionGoToLabel     = 0x8c, // name (STR)
		
		// Flash 4
		ActionAdd		   = 0x0A, // Stack IN: number, number, OUT: number
		ActionSubtract	   = 0x0B, // Stack IN: number, number, OUT: number
		ActionMultiply	   = 0x0C, // Stack IN: number, number, OUT: number
		ActionDivide	   = 0x0D, // Stack IN: dividend, divisor, OUT: number
		ActionEquals	   = 0x0E, // Stack IN: number, number, OUT: bool
		ActionLess		   = 0x0F, // Stack IN: number, number, OUT: bool
		ActionAnd		   = 0x10, // Stack IN: bool, bool, OUT: bool
		ActionOr		   = 0x11, // Stack IN: bool, bool, OUT: bool
		ActionNot		   = 0x12, // Stack IN: bool, OUT: bool
		ActionStringEquals = 0x13, // Stack IN: string, string, OUT: bool
		ActionStringLength = 0x14, // Stack IN: string, OUT: number
		ActionStringAdd	   = 0x21, // Stack IN: string, strng, OUT: string
		ActionStringExtract= 0x15, // Stack IN: string, index, count, OUT: substring
		ActionPush		   = 0x96, // type (BYTE), value (STRING or FLOAT)
		ActionPop		   = 0x17, // no arguments
		ActionToInteger	   = 0x18, // Stack IN: number, OUT: integer
		ActionJump		   = 0x99, // offset (WORD)
		ActionIf		   = 0x9D, // offset (WORD) Stack IN: bool
		ActionCall		   = 0x9E, // Stack IN: name
		ActionGetVariable  = 0x1C, // Stack IN: name, OUT: value
		ActionSetVariable  = 0x1D, // Stack IN: name, value
		ActionGetURL2	   = 0x9A, // method (BYTE) Stack IN: url, window
		ActionGotoFrame2   = 0x9F, // flags (BYTE) Stack IN: frame
		ActionSetTarget2   = 0x20, // Stack IN: target
		ActionGetProperty  = 0x22, // Stack IN: target, property, OUT: value
		ActionSetProperty  = 0x23, // Stack IN: target, property, value
	    ActionCloneSprite  = 0x24, // Stack IN: source, target, depth
	    ActionRemoveSprite = 0x25, // Stack IN: target
		ActionTrace        = 0x26, // Stack IN: message
		ActionStartDrag	   = 0x27, // Stack IN: no constraint: 0, center, target
		                            //           constraint: x1, y1, x2, y2, 1, center, target
		ActionEndDrag	   = 0x28, // no arguments
		ActionStringLess   = 0x29, // Stack IN: string, string, OUT: bool
		ActionWaitForFrame2= 0x8D, // skipCount (BYTE) Stack IN: frame
	    ActionRandomNumber = 0x30, // Stack IN: maximum, OUT: result
		ActionMBStringLength = 0x31, // Stack IN: string, OUT: length
		ActionCharToAscii  = 0x32, // Stack IN: character, OUT: ASCII code
		ActionAsciiToChar  = 0x33, // Stack IN: ASCII code, OUT: character
		ActionGetTime	   = 0x34, // Stack OUT: milliseconds since Player start
		ActionMBStringExtract = 0x35,// Stack IN: string, index, count, OUT: substring
		ActionMBCharToAscii   = 0x36,// Stack IN: character, OUT: ASCII code
		ActionMBAsciiToChar   = 0x37,// Stack IN: ASCII code, OUT: character

		// Flash 5
		ActionCallFunction = 0x3d,
		ActionCallMethod   = 0x52,
		ActionConstantPool = 0x88,
		ActionDefineFunction = 0x9b,
		ActionDefineLocal = 0x3c,
		ActionDefineLocal2 = 0x41,
		ActionDelete = 0x3a,
		ActionDelete2 = 0x3b,
		ActionEnumerate = 0x46,
		ActionEquals2 = 0x49,
		ActionGetMember = 0x4e,
		ActionInitArray = 0x42,
		//ActionInitObject = 0x42, // TOFIX: brr, which value is correct ?
		ActionNewMethod = 0x53,
		ActionNewObject = 0x40,
		ActionSetMember = 0x4f,
		ActionTargetPath = 0x45,
		ActionWith = 0x94,
		ActionToNumber = 0x4a,
		ActionToString = 0x4b,
		ActionTypeOf = 0x44,
		ActionAdd2 = 0x47,
		ActionLess2 = 0x48,
		ActionModulo = 0x3f,
		ActionBitAnd = 0x60,
		ActionBitLShift = 0x63,
		ActionBitOr = 0x61,
		ActionBitRShift = 0x64,
		ActionBitURShift = 0x65,
		ActionBitXor = 0x62,
		ActionDecrement = 0x51,
		ActionIncrement = 0x50,
		ActionPushDuplicate = 0x4c,
		ActionReturn = 0x3e,
		ActionStackSwap = 0x4d,
		ActionStoreRegister = 0x87,


		// Reserved for Quicktime
		ActionQuickTime	   = 0xAA  // I think this is what they are using...
};

class Sound;

struct ActionRecord {
	Action			 action;

	// GotoFrame  & WaitForFrame
	long			 frameIndex;

	// GetURL
	char			*url;
	char			*target;

	// GotoLabel
	char			*frameLabel;

	// WaitForFrame
	long			 skipCount;

	// Sound
	Sound			*sound;

	// Stack TOFIX: all this is lame. There should be a general structure casted depending on what the functions are
	long stack_type;
	char *stack_string;
	FLOAT stack_float;

	struct ActionRecord	*next;

	ActionRecord() {
		frameLabel = 0;
		url = 0;
		target = 0;
		sound = 0;
	};

	~ActionRecord() {
		if (frameLabel) free(frameLabel);
		if (url) free(url);
		if (target) free(target);
	};
};

enum FontFlags {
	fontUnicode   = 0x20,
	fontShiftJIS  = 0x10,
	fontANSI      = 0x08,
	fontItalic    = 0x04,
	fontBold      = 0x02,
	fontWideCodes = 0x01
};

enum TextFlags {
	isTextControl = 0x80,

	textIsLarge   = 0x70,
	textHasFont   = 0x08,
	textHasColor  = 0x04,
	textHasYOffset= 0x02,
	textHasXOffset= 0x01
};

#ifndef NULL
#define NULL 0
#endif

// Tag values that represent actions or data in a Flash script.
enum
{
	stagEnd 			= 0,
	stagShowFrame 		= 1,
	stagDefineShape		= 2,
	stagFreeCharacter 		= 3,
	stagPlaceObject 		= 4,
	stagRemoveObject 		= 5,
	stagDefineBits 		= 6,
	stagDefineButton 		= 7,
	stagJPEGTables 		= 8,
	stagSetBackgroundColor	= 9,
	stagDefineFont		= 10,
	stagDefineText		= 11,
	stagDoAction		= 12,
	stagDefineFontInfo		= 13,
	stagDefineSound		= 14,	// Event sound tags.
	stagStartSound		= 15,
	stagStopSound		= 16,
	stagDefineButtonSound	= 17,
	stagSoundStreamHead		= 18,
	stagSoundStreamBlock	= 19,
	stagDefineBitsLossless	= 20,	// A bitmap using lossless zlib compression.
	stagDefineBitsJPEG2		= 21,	// A bitmap using an internal JPEG compression table.
	stagDefineShape2		= 22,
	stagDefineButtonCxform	= 23,
	stagProtect			= 24,	// This file should not be importable for editing.

	// These are the new tags for Flash 3.
	stagPlaceObject2		= 26,	// The new style place w/ alpha color transform and name.
	stagRemoveObject2		= 28,	// A more compact remove object that omits the character tag (just depth).
	stagDefineShape3		= 32,	// A shape V3 includes alpha values.
	stagDefineText2		= 33,	// A text V2 includes alpha values.
	stagDefineButton2		= 34,	// A button V2 includes color transform, alpha and multiple actions
	stagDefineBitsJPEG3		= 35,	// A JPEG bitmap with alpha info.
	stagDefineBitsLossless2 	= 36,	// A lossless bitmap with alpha info.
	stagDefineSprite		= 39,	// Define a sequence of tags that describe the behavior of a sprite.
	stagNameCharacter		= 40,	// Name a character definition, character id and a string, (used for buttons, bitmaps, sprites and sounds).
	stagFrameLabel			= 43,	// A string label for the current frame.
	stagSoundStreamHead2	= 45,	// For lossless streaming sound, should not have needed this...
	stagDefineMorphShape	= 46,	// A morph shape definition
	stagDefineFont2		= 48,
	/* new tags for Flash4+ */
	stagGenCommand			= 49,	// a tag command for the Flash Generator intrinsic
	stagDefineCommandObj	= 50,	// a tag command for the Flash Generator intrinsic Command
	stagCharacterSet		= 51,	// defines the character set used to store strings
	stagFontRef				= 52,   // defines a reference to an external font source

	// Flash 4 tags
	stagDefineEditText		= 37,	// an edit text object (bounds, width, font, variable name)
	stagDefineVideo			= 38,	// a reference to an external video stream

	// NOTE: If tag values exceed 255 we need to expand SCharacter::tagCode from a BYTE to a WORD
	stagDefineBitsPtr		= 1023,  // a special tag used only in the editor

	notEnoughData		= 0xffff	// Special code
};

#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif

extern "C"
{
extern int shape_size,shape_nb,shaperecord_size,shaperecord_nb,style_size,style_nb;
}
typedef void (*ScanLineFunc)(void *id, long y, long start, long end);

class Bitmap;
struct FlashMovie;


extern "C" {
#include "Jpeg/jpeglib.h"
};
extern "C" {
#ifdef MBX
#include <z_lib_calls.h>
extern ZBASE;

#else
#include "Zlib/zlib.h"
#endif
};

#include "graphic.h"
#include "character.h"
#include "bitmap.h"
#include "shape.h"
#include "displaylist.h"
#include "sound.h"
#include "button.h"
#include "font.h"
#include "text.h"
#include "adpcm.h"
#include "program.h"
#include "sprite.h"
#include "script.h"
#include "movie.h"

#endif /* _SWF_H_ */
