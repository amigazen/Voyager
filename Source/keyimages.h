/**************************************************************************

  =======================
  The Voyager Web Browser
  =======================

  Copyright (C) 1995-2001 by
   Oliver Wagner <owagner@vapor.com>
   All Rights Reserved

  Parts Copyright (C) by
   David Gerber <zapek@vapor.com>
   Jon Bright <jon@siliconcircus.com>
   Matt Sealey <neko@vapor.com>

**************************************************************************/


#ifndef VOYAGER_KEYIMAGES_H
#define VOYAGER_KEYIMAGES_H
/*
 * $Id: keyimages.h,v 1.9 2001/07/01 22:02:58 owagner Exp $
 */

/** "keyimage1" created by ShapeMe 1.6 ((7.12.96)) from "keyimage1.iff"; size 13/6/3 **/ 

/* RGB32 ColorMap for "keyimage1" */
static ULONG keyimage1CMap32[]={
	0x96969696,0x96969696,0x96969696,
	0x00000000,0x00000000,0x00000000,
	0xFFFFFFFF,0x3D3D3D3D,0x45454545,
	0x3D3D3D3D,0x65656565,0xA2A2A2A2,
	0x79797979,0x79797979,0x79797979,
	0xAEAEAEAE,0xAEAEAEAE,0xAEAEAEAE,
	0xC2C2C2C2,0xC2C2C2C2,0x04040404,
	0xFFFFFFFF,0xFFFFFFFF,0x00000000
};

/* BitPlane Data for "keyimage1" */
static USHORT __chip keyimage1Data[18]={
	/* BitPlane 0 */
	0x7000,0xb800,0x8fe0,0x8b50,0x74a0,0x3800,
	/* BitPlane 1 */
	0x7000,0x8800,0x8fe0,0x8940,0x7000,0x0000,
	/* BitPlane 2 */
	0x7800,0xcc00,0xcff0,0xcde0,0x7800,0x0000};

/* BitMap for "keyimage1" */
static struct BitMap keyImage1BitMap={2,6,0,3,0,(PLANEPTR)&keyimage1Data[0],(PLANEPTR)&keyimage1Data[6],(PLANEPTR)&keyimage1Data[12]};

/**** end of "keyimage1" ****/

/** "keyimage2" created by ShapeMe 1.6 ((7.12.96)) from "keyimage2.iff"; size 13/6/3 **/ 

/* BitPlane Data for "keyimage2" */
static USHORT __chip keyimage2Data[18]={
	/* BitPlane 0 */
	0x0020,0x3020,0x0040,0x0200,0x0408,0x38d0,
	/* BitPlane 1 */
	0x7080,0x8880,0x8f00,0x89f0,0x72a0,0x0200,
	/* BitPlane 2 */
	0x7840,0xcc40,0xce80,0xccf8,0x79f0,0x0100};

/* BitMap for "keyimage2" */
static struct BitMap keyImage2BitMap={2,6,0,3,0,(PLANEPTR)&keyimage2Data[0],(PLANEPTR)&keyimage2Data[6],(PLANEPTR)&keyimage2Data[12]};

/**** end of "keyimage2" ****/

/** "http10img" created by ShapeMe 1.6 ((7.12.96)) from "http10img.iff"; size 8/6/1 **/ 

/* RGB32 ColorMap for "http10img" */
static ULONG http10imgCMap32[]={
	0x96969696,0x96969696,0x96969696,
	0x00000000,0x00000000,0x00000000
};

/* BitPlane Data for "http10img" */
static USHORT __chip http10imgData[6]={
	/* BitPlane 0 */
	0x4200,0xc500,0x4500,0x4500,0x4500,0x5200};

/* BitMap for "http10img" */
static struct BitMap http10ImageBitMap={2,6,0,1,0,(PLANEPTR)&http10imgData[0]};

/**** end of "http10img" ****/

/** "http11img" created by ShapeMe 1.6 ((7.12.96)) from "http11img.iff"; size 8/6/1 **/ 

/* BitPlane Data for "http11img" */
static USHORT __chip http11imgData[6]={
	/* BitPlane 0 */
	0x2200,0x6600,0x2200,0x2200,0x2200,0x2a00};

/* BitMap for "http11img" */
static struct BitMap http11ImageBitMap={2,6,0,1,0,(PLANEPTR)&http11imgData[0]};

/**** end of "http11img" ****/

/** "ftpimg" created by ShapeMe 1.6 ((7.12.96)) from "ftpimg.iff"; size 8/6/1 **/ 

/* BitPlane Data for "ftpimg" */
static USHORT __chip ftpimgData[6]={
	/* BitPlane 0 */
	0xff00,0x9500,0xd700,0x9400,0x9400,0x0000};

/* BitMap for "ftpimg" */
static struct BitMap ftpImageBitMap={2,6,0,1,0,(PLANEPTR)&ftpimgData[0]};

/**** end of "ftpimg" ****/

#endif /* VOYAGER_KEYIMAGES_H */
