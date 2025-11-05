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
** $Id: colortable.c,v 1.23 2003/07/06 16:51:33 olli Exp $
**
*/

#include "voyager.h"

/* public */
#if defined( AMIGAOS ) || defined( __MORPHOS__ )
#include <proto/graphics.h>
#endif

/* private */
#include "mui_func.h"

#define NUMCOLS 676

static ULONG __far color_hash[ 676 ] = {
	0x2491af8, // aqua
	0x294f90e, // navy
	0xbe6c941f, // olive
	0x21d14b23, // fuchsia
	0x363653d5, // silver
	0x2b8e3de, // teal
	0x289bdbf, // lime
	0xa6ba09e6, // alice
	0xaaaf0d30, // AntiqueWhite
	0xabeac2e1, // AntiqueWhite1
	0xabeac2e2, // AntiqueWhite2
	0xabeac2e3, // AntiqueWhite3
	0xabeac2e4, // AntiqueWhite4
	0xf8da904c, // aquamarine
	0xf65325dd, // aquamarine1
	0xf65325de, // aquamarine2
	0xf65325df, // aquamarine3
	0xf65325e0, // aquamarine4
	0xa70e2257, // azure
	0xa307cb00, // azure1
	0xa307cb01, // azure2
	0xa307cb02, // azure3
	0xa307cb03, // azure4
	0xa841d07c, // beige
	0x1c5eef9, // bisque
	0x81712532, // bisque1
	0x81712533, // bisque2
	0x81712534, // bisque3
	0x81712535, // bisque4
	0xa86ab605, // black
	0xe8461639, // blanched
	0x24ea280, // blue
	0xa86c56b1, // blue1
	0xa86c56b2, // blue2
	0xa86c56b3, // blue3
	0xa86c56b4, // blue4
	0xa88f7cc0, // brown
	0x10ea92f1, // brown1
	0x10ea92f2, // brown2
	0x10ea92f3, // brown3
	0x10ea92f4, // brown4
	0xf89d096f, // burlywood
	0xe4c7b0d8, // burlywood1
	0xe4c7b0d9, // burlywood2
	0xe4c7b0da, // burlywood3
	0xe4c7b0db, // burlywood4
	0xa9dafc21, // cadet
	0x41223fba, // CadetBlue1
	0x41223fbb, // CadetBlue2
	0x41223fbc, // CadetBlue3
	0x41223fbd, // CadetBlue4
	0x414708a6, // chartreuse
	0x9d417787, // chartreuse1
	0x9d417788, // chartreuse2
	0x9d417789, // chartreuse3
	0x9d41778a, // chartreuse4
	0x3a8abf6a, // chocolate
	0xb190956b, // chocolate1
	0xb190956c, // chocolate2
	0xb190956d, // chocolate3
	0xb190956e, // chocolate4
	0xaa2f38c1, // coral
	0x87772f3a, // coral1
	0x87772f3b, // coral2
	0x87772f3c, // coral3
	0x87772f3d, // coral4
	0xd02c1b99, // cornflower
	0xfe2d82a5, // cornsilk
	0x7afa413e, // cornsilk1
	0x7afa413f, // cornsilk2
	0x7afa4140, // cornsilk3
	0x7afa4141, // cornsilk4
	0x2559b0b, // cyan
	0xaa693654, // cyan1
	0xaa693655, // cyan2
	0xaa693656, // cyan3
	0xaa693657, // cyan4
	0xd9da0fa2, // darkblue
	0xd9e1082d, // darkcyan
	0x6c9c87e0, // darkgoldenrod
	0x27c7634b, // darkgreen
	0xd9f83609, // darkgrey
	0xd9f834e5, // darkgray
	0x2e510042, // darkkhaki
	0x45a95d1f, // darkmagenta
	0x352eb2d1, // darkolive
	0x3448cb06, // darkorange
	0x34542de3, // darkorchid
	0x1b898a55, // darkred
	0x605c70c, // darksalmon
	0x1b899f23, // darksea
	0x3bf3563b, // darkslate
	0x57ea8e73, // darkturquoise
	0x8653c2f5, // darkviolet
	0xf8a2bf11, // DarkGoldenrod1
	0xf8a2bf12, // DarkGoldenrod2
	0xf8a2bf13, // DarkGoldenrod3
	0xf8a2bf14, // DarkGoldenrod4
	0x64efd713, // DarkOliveGreen1
	0x64efd714, // DarkOliveGreen2
	0x64efd715, // DarkOliveGreen3
	0x64efd716, // DarkOliveGreen4
	0xe8c1e4e7, // DarkOrange1
	0xe8c1e4e8, // DarkOrange2
	0xe8c1e4e9, // DarkOrange3
	0xe8c1e4ea, // DarkOrange4
	0xec0115ec, // DarkOrchid1
	0xec0115ed, // DarkOrchid2
	0xec0115ee, // DarkOrchid3
	0xec0115ef, // DarkOrchid4
	0x1ba30945, // DarkSeaGreen1
	0x1ba30946, // DarkSeaGreen2
	0x1ba30947, // DarkSeaGreen3
	0x1ba30948, // DarkSeaGreen4
	0x8d305da8, // deeppink
	0xf3e85c65, // deepsky
	0x42cab519, // DeepPink1
	0x42cab51a, // DeepPink2
	0x42cab51b, // DeepPink3
	0x42cab51c, // DeepPink4
	0xf254d99e, // DeepSkyBlue1
	0xf254d99f, // DeepSkyBlue2
	0xf254d9a0, // DeepSkyBlue3
	0xf254d9a1, // DeepSkyBlue4
	0xef7fae29, // dimgrey
	0xef7fad05, // dimgray
	0x2b51fed, // dodger
	0x206bc866, // DodgerBlue1
	0x206bc867, // DodgerBlue2
	0x206bc868, // DodgerBlue3
	0x206bc869, // DodgerBlue4
	0xdaa655a9, // firebrick
	0x596e6d62, // firebrick1
	0x596e6d63, // firebrick2
	0x596e6d64, // firebrick3
	0x596e6d65, // firebrick4
	0x1f8cb371, // floralwhite
	0x306bd8c, // forestgreen
	0x6e5213ac, // gainsboro
	0xb0cabccd, // ghost
	0x26c8c5e, // gold
	0xb0f406ff, // gold1
	0xb0f40700, // gold2
	0xb0f40701, // gold3
	0xb0f40702, // gold4
	0xa3a035ee, // goldenrod
	0xa8af610f, // goldenrod1
	0xa8af6110, // goldenrod2
	0xa8af6111, // goldenrod3
	0xa8af6112, // goldenrod4
	0xb1054499, // green
	0x7a808fd2, // green1
	0x7a808fd3, // green2
	0x7a808fd4, // green3
	0x7a808fd5, // green4
	0x26cc8e7, // grey
	0xb1054a0f, // grey0
	0xb1054a10, // grey1
	0xb1054a11, // grey2
	0xb1054a12, // grey3
	0xb1054a13, // grey4
	0xb1054a14, // grey5
	0xb1054a15, // grey6
	0xb1054a16, // grey7
	0xb1054a17, // grey8
	0xb1054a18, // grey9
	0x7a821ec0, // grey10
	0x7a821ec1, // grey11
	0x7a821ec2, // grey12
	0x7a821ec3, // grey13
	0x7a821ec4, // grey14
	0x7a821ec5, // grey15
	0x7a821ec6, // grey16
	0x7a821ec7, // grey17
	0x7a821ec8, // grey18
	0x7a821ec9, // grey19
	0x7a821f09, // grey20
	0x7a821f0a, // grey21
	0x7a821f0b, // grey22
	0x7a821f0c, // grey23
	0x7a821f0d, // grey24
	0x7a821f0e, // grey25
	0x7a821f0f, // grey26
	0x7a821f10, // grey27
	0x7a821f11, // grey28
	0x7a821f12, // grey29
	0x7a821f52, // grey30
	0x7a821f53, // grey31
	0x7a821f54, // grey32
	0x7a821f55, // grey33
	0x7a821f56, // grey34
	0x7a821f57, // grey35
	0x7a821f58, // grey36
	0x7a821f59, // grey37
	0x7a821f5a, // grey38
	0x7a821f5b, // grey39
	0x7a821f9b, // grey40
	0x7a821f9c, // grey41
	0x7a821f9d, // grey42
	0x7a821f9e, // grey43
	0x7a821f9f, // grey44
	0x7a821fa0, // grey45
	0x7a821fa1, // grey46
	0x7a821fa2, // grey47
	0x7a821fa3, // grey48
	0x7a821fa4, // grey49
	0x7a821fe4, // grey50
	0x7a821fe5, // grey51
	0x7a821fe6, // grey52
	0x7a821fe7, // grey53
	0x7a821fe8, // grey54
	0x7a821fe9, // grey55
	0x7a821fea, // grey56
	0x7a821feb, // grey57
	0x7a821fec, // grey58
	0x7a821fed, // grey59
	0x7a82202d, // grey60
	0x7a82202e, // grey61
	0x7a82202f, // grey62
	0x7a822030, // grey63
	0x7a822031, // grey64
	0x7a822032, // grey65
	0x7a822033, // grey66
	0x7a822034, // grey67
	0x7a822035, // grey68
	0x7a822036, // grey69
	0x7a822076, // grey70
	0x7a822077, // grey71
	0x7a822078, // grey72
	0x7a822079, // grey73
	0x7a82207a, // grey74
	0x7a82207b, // grey75
	0x7a82207c, // grey76
	0x7a82207d, // grey77
	0x7a82207e, // grey78
	0x7a82207f, // grey79
	0x7a8220bf, // grey80
	0x7a8220c0, // grey81
	0x7a8220c1, // grey82
	0x7a8220c2, // grey83
	0x7a8220c3, // grey84
	0x7a8220c4, // grey85
	0x7a8220c5, // grey86
	0x7a8220c6, // grey87
	0x7a8220c7, // grey88
	0x7a8220c8, // grey89
	0x7a822108, // grey90
	0x7a822109, // grey91
	0x7a82210a, // grey92
	0x7a82210b, // grey93
	0x7a82210c, // grey94
	0x7a82210d, // grey95
	0x7a82210e, // grey96
	0x7a82210f, // grey97
	0x7a822110, // grey98
	0x7a822111, // grey99
	0xef1ac4f0, // grey100
	0x26cc7c3, // gray
	0xb104f6cb, // gray0
	0xb104f6cc, // gray1
	0xb104f6cd, // gray2
	0xb104f6ce, // gray3
	0xb104f6cf, // gray4
	0xb104f6d0, // gray5
	0xb104f6d1, // gray6
	0xb104f6d2, // gray7
	0xb104f6d3, // gray8
	0xb104f6d4, // gray9
	0x7a6a605c, // gray10
	0x7a6a605d, // gray11
	0x7a6a605e, // gray12
	0x7a6a605f, // gray13
	0x7a6a6060, // gray14
	0x7a6a6061, // gray15
	0x7a6a6062, // gray16
	0x7a6a6063, // gray17
	0x7a6a6064, // gray18
	0x7a6a6065, // gray19
	0x7a6a60a5, // gray20
	0x7a6a60a6, // gray21
	0x7a6a60a7, // gray22
	0x7a6a60a8, // gray23
	0x7a6a60a9, // gray24
	0x7a6a60aa, // gray25
	0x7a6a60ab, // gray26
	0x7a6a60ac, // gray27
	0x7a6a60ad, // gray28
	0x7a6a60ae, // gray29
	0x7a6a60ee, // gray30
	0x7a6a60ef, // gray31
	0x7a6a60f0, // gray32
	0x7a6a60f1, // gray33
	0x7a6a60f2, // gray34
	0x7a6a60f3, // gray35
	0x7a6a60f4, // gray36
	0x7a6a60f5, // gray37
	0x7a6a60f6, // gray38
	0x7a6a60f7, // gray39
	0x7a6a6137, // gray40
	0x7a6a6138, // gray41
	0x7a6a6139, // gray42
	0x7a6a613a, // gray43
	0x7a6a613b, // gray44
	0x7a6a613c, // gray45
	0x7a6a613d, // gray46
	0x7a6a613e, // gray47
	0x7a6a613f, // gray48
	0x7a6a6140, // gray49
	0x7a6a6180, // gray50
	0x7a6a6181, // gray51
	0x7a6a6182, // gray52
	0x7a6a6183, // gray53
	0x7a6a6184, // gray54
	0x7a6a6185, // gray55
	0x7a6a6186, // gray56
	0x7a6a6187, // gray57
	0x7a6a6188, // gray58
	0x7a6a6189, // gray59
	0x7a6a61c9, // gray60
	0x7a6a61ca, // gray61
	0x7a6a61cb, // gray62
	0x7a6a61cc, // gray63
	0x7a6a61cd, // gray64
	0x7a6a61ce, // gray65
	0x7a6a61cf, // gray66
	0x7a6a61d0, // gray67
	0x7a6a61d1, // gray68
	0x7a6a61d2, // gray69
	0x7a6a6212, // gray70
	0x7a6a6213, // gray71
	0x7a6a6214, // gray72
	0x7a6a6215, // gray73
	0x7a6a6216, // gray74
	0x7a6a6217, // gray75
	0x7a6a6218, // gray76
	0x7a6a6219, // gray77
	0x7a6a621a, // gray78
	0x7a6a621b, // gray79
	0x7a6a625b, // gray80
	0x7a6a625c, // gray81
	0x7a6a625d, // gray82
	0x7a6a625e, // gray83
	0x7a6a625f, // gray84
	0x7a6a6260, // gray85
	0x7a6a6261, // gray86
	0x7a6a6262, // gray87
	0x7a6a6263, // gray88
	0x7a6a6264, // gray89
	0x7a6a62a4, // gray90
	0x7a6a62a5, // gray91
	0x7a6a62a6, // gray92
	0x7a6a62a7, // gray93
	0x7a6a62a8, // gray94
	0x7a6a62a9, // gray95
	0x7a6a62aa, // gray96
	0x7a6a62ab, // gray97
	0x7a6a62ac, // gray98
	0x7a6a62ad, // gray99
	0xe8557a6c, // gray100
	0xde250503, // honeydew
	0x588e6e0c, // honeydew1
	0x588e6e0d, // honeydew2
	0x588e6e0e, // honeydew3
	0x588e6e0f, // honeydew4
	0x89503, // hot
	0xd5c8c96e, // HotPink1
	0xd5c8c96f, // HotPink2
	0xd5c8c970, // HotPink3
	0xd5c8c971, // HotPink4
	0x6ad6c553, // indian
	0xc056b47f, // IndianRed1
	0xc056b480, // IndianRed2
	0xc056b481, // IndianRed3
	0xc056b482, // IndianRed4
	0xb4807c29, // ivory
	0x78a367e2, // ivory1
	0x78a367e3, // ivory2
	0x78a367e4, // ivory3
	0x78a367e5, // ivory4
	0xb78ee190, // khaki
	0x57be5241, // khaki1
	0x57be5242, // khaki2
	0x57be5243, // khaki3
	0x57be5244, // khaki4
	0x20587969, // lavender
	0xe993420, // LavenderBlush1
	0xe993421, // LavenderBlush2
	0xe993422, // LavenderBlush3
	0xe993423, // LavenderBlush4
	0xc2028c43, // lawngreen
	0x9e642f8, // lemonchiffon
	0xd2a918e9, // LemonChiffon1
	0xd2a918ea, // LemonChiffon2
	0xd2a918eb, // LemonChiffon3
	0xd2a918ec, // LemonChiffon4
	0x90ceee60, // lightblue
	0x4cc4dba1, // lightcoral
	0x90d5e6eb, // lightcyan
	0x4ec764ce, // lightgoldenrod
	0x539ae779, // lightgreen
	0x90ed14c7, // lightgrey
	0x90ed13a3, // lightgray
	0x9121c852, // lightpink
	0x8556782a, // lightsalmon
	0xc9e123f1, // lightsea
	0xc9e125bf, // lightsky
	0x67c6da69, // lightslate
	0x67f6a635, // lightsteel
	0x717f0d5c, // lightyellow
	0x4b01f991, // LightBlue1
	0x4b01f992, // LightBlue2
	0x4b01f993, // LightBlue3
	0x4b01f994, // LightBlue4
	0x4cfed934, // LightCyan1
	0x4cfed935, // LightCyan2
	0x4cfed936, // LightCyan3
	0x4cfed937, // LightCyan4
	0x76dbbeef, // LightGoldenrod1
	0x76dbbef0, // LightGoldenrod2
	0x76dbbef1, // LightGoldenrod3
	0x76dbbef2, // LightGoldenrod4
	0x62a21f93, // LightPink1
	0x62a21f94, // LightPink2
	0x62a21f95, // LightPink3
	0x62a21f96, // LightPink4
	0x5a8442b, // LightSalmon1
	0x5a8442c, // LightSalmon2
	0x5a8442d, // LightSalmon3
	0x5a8442e, // LightSalmon4
	0x2add6288, // LightSkyBlue1
	0x2add6289, // LightSkyBlue2
	0x2add628a, // LightSkyBlue3
	0x2add628b, // LightSkyBlue4
	0x39137ee, // LightSteelBlue1
	0x39137ef, // LightSteelBlue2
	0x39137f0, // LightSteelBlue3
	0x39137f1, // LightSteelBlue4
	0x5d3acf6d, // LightYellow1
	0x5d3acf6e, // LightYellow2
	0x5d3acf6f, // LightYellow3
	0x5d3acf70, // LightYellow4
	0x71d0a870, // limegreen
	0xb94730b6, // linen
	0x26c865cd, // magenta
	0xf2507a6, // magenta1
	0xf2507a7, // magenta2
	0xf2507a8, // magenta3
	0xf2507a9, // magenta4
	0x436b6e0c, // maroon
	0x39a2619d, // maroon1
	0x39a2619e, // maroon2
	0x39a2619f, // maroon3
	0x39a261a0, // maroon4
	0x2e36055d, // mediumaquamarine
	0x8048f821, // mediumblue
	0xbf09e992, // mediumorchid
	0x400822b9, // mediumpurple
	0x47e651aa, // mediumsea
	0xb193a272, // mediumslate
	0xaa42a42c, // mediumspring
	0x932b1b0a, // mediumturquoise
	0x11097ea4, // mediumviolet
	0x79d39ad3, // MediumOrchid1
	0x79d39ad4, // MediumOrchid2
	0x79d39ad5, // MediumOrchid3
	0x79d39ad6, // MediumOrchid4
	0x4251e6f2, // MediumPurple1
	0x4251e6f3, // MediumPurple2
	0x4251e6f4, // MediumPurple3
	0x4251e6f5, // MediumPurple4
	0x8d58d0c4, // midnight
	0x840a5e20, // mintcream
	0xd6657c6f, // mistyrose
	0x22f07bd8, // MistyRose1
	0x22f07bd9, // MistyRose2
	0x22f07bda, // MistyRose3
	0x22f07bdb, // MistyRose4
	0x685169dd, // moccasin
	0x52db3dc8, // navajowhite
	0xa0849e39, // NavajoWhite1
	0xa0849e3a, // NavajoWhite2
	0xa0849e3b, // NavajoWhite3
	0xa0849e3c, // NavajoWhite4
	0x9a73c04e, // navyblue
	0xe985298c, // oldlace
	0xd9f11991, // OliveDrab1
	0xd9f11992, // OliveDrab2
	0xd9f11993, // OliveDrab3
	0xd9f11994, // OliveDrab4
	0x56ee0a44, // orange
	0x633700e7, // orangered
	0xc9e0ed95, // orange1
	0xc9e0ed96, // orange2
	0xc9e0ed97, // orange3
	0xc9e0ed98, // orange4
	0x4aaf4210, // OrangeRed1
	0x4aaf4211, // OrangeRed2
	0x4aaf4212, // OrangeRed3
	0x4aaf4213, // OrangeRed4
	0x56f96d21, // orchid
	0xcd201e9a, // orchid1
	0xcd201e9b, // orchid2
	0xcd201e9c, // orchid3
	0xcd201e9d, // orchid4
	0x47d494d0, // palegoldenrod
	0x50b03a3b, // palegreen
	0x33229b63, // paleturquoise
	0x30b90d65, // paleviolet
	0x2409b04, // PaleGreen1
	0x2409b05, // PaleGreen2
	0x2409b06, // PaleGreen3
	0x2409b07, // PaleGreen4
	0x94de4f6c, // PaleTurquoise1
	0x94de4f6d, // PaleTurquoise2
	0x94de4f6e, // PaleTurquoise3
	0x94de4f6f, // PaleTurquoise4
	0x952dcbd1, // PaleVioletRed1
	0x952dcbd2, // PaleVioletRed2
	0x952dcbd3, // PaleVioletRed3
	0x952dcbd4, // PaleVioletRed4
	0x5639f1dc, // papayawhip
	0x75d13832, // peachpuff
	0x98a90673, // PeachPuff1
	0x98a90674, // PeachPuff2
	0x98a90675, // PeachPuff3
	0x98a90676, // PeachPuff4
	0x2a12a5c, // peru
	0x2a17c72, // pink
	0xc00c7cb3, // pink1
	0xc00c7cb4, // pink2
	0xc00c7cb5, // pink3
	0xc00c7cb6, // pink4
	0x2a1bce6, // plum
	0xc01eddc7, // plum1
	0xc01eddc8, // plum2
	0xc01eddc9, // plum3
	0xc01eddca, // plum4
	0x6bfb55e1, // powderblue
	0xd7f7a648, // purple
	0x959e6ab9, // purple1
	0x959e6aba, // purple2
	0x959e6abb, // purple3
	0x959e6abc, // purple4
	0x96243, // red
	0x2ad054c, // red1
	0x2ad054d, // red2
	0x2ad054e, // red3
	0x2ad054f, // red4
	0x3f63b64d, // rosybrown
	0x136efc26, // RosyBrown1
	0x136efc27, // RosyBrown2
	0x136efc28, // RosyBrown3
	0x136efc29, // RosyBrown4
	0x23ba72c7, // royalblue
	0x302abaf0, // RoyalBlue1
	0x302abaf1, // RoyalBlue2
	0x302abaf2, // RoyalBlue3
	0x302abaf3, // RoyalBlue4
	0xb6cddb8d, // saddlebrown
	0x28ab064a, // salmon
	0x98c4cb4b, // salmon1
	0x98c4cb4c, // salmon2
	0x98c4cb4d, // salmon3
	0x98c4cb4e, // salmon4
	0xfb4650af, // sandybrown
	0x27b52212, // seagreen
	0x52a6b753, // SeaGreen1
	0x52a6b754, // SeaGreen2
	0x52a6b755, // SeaGreen3
	0x52a6b756, // SeaGreen4
	0x3bc9a7a1, // seashell
	0xc80cd1a, // seashell1
	0xc80cd1b, // seashell2
	0xc80cd1c, // seashell3
	0xc80cd1d, // seashell4
	0x360c229e, // sienna
	0x6975df3f, // sienna1
	0x6975df40, // sienna2
	0x6975df41, // sienna3
	0x6975df42, // sienna4
	0x822a14bf, // skyblue
	0x1dffeaa8, // SkyBlue1
	0x1dffeaa9, // SkyBlue2
	0x1dffeaaa, // SkyBlue3
	0x1dffeaab, // SkyBlue4
	0xb8c571a9, // slateblue
	0xb8e39810, // slategrey
	0xb8e396ec, // slategray
	0xb04d6962, // SlateBlue1
	0xb04d6963, // SlateBlue2
	0xb04d6964, // SlateBlue3
	0xb04d6965, // SlateBlue4
	0x2b3b3a7, // snow
	0xc53e3ad0, // snow1
	0xc53e3ad1, // snow2
	0xc53e3ad2, // snow3
	0xc53e3ad3, // snow4
	0x270ce2cc, // springgreen
	0x22acac5d, // SpringGreen1
	0x22acac5e, // SpringGreen2
	0x22acac5f, // SpringGreen3
	0x22acac60, // SpringGreen4
	0xda27f4f5, // steelblue
	0x3564da0e, // SteelBlue1
	0x3564da0f, // SteelBlue2
	0x3564da10, // SteelBlue3
	0x3564da11, // SteelBlue4
	0x98acb, // tan
	0x2b89414, // tan1
	0x2b89415, // tan2
	0x2b89416, // tan3
	0x2b89417, // tan4
	0x3101d185, // thistle
	0xf984bf1e, // thistle1
	0xf984bf1f, // thistle2
	0xf984bf20, // thistle3
	0xf984bf21, // thistle4
	0xbbf3123c, // tomato
	0x9850334d, // tomato1
	0x9850334e, // tomato2
	0x9850334f, // tomato3
	0x98503350, // tomato4
	0x8eee3c81, // turquoise
	0xc1ef40fa, // turquoise1
	0xc1ef40fb, // turquoise2
	0xc1ef40fc, // turquoise3
	0xc1ef40fd, // turquoise4
	0xa8f90233, // violet
	0x80e84fbe, // violetred
	0xc23ebd5f, // VioletRed1
	0xc23ebd60, // VioletRed2
	0xc23ebd61, // VioletRed3
	0xc23ebd62, // VioletRed4
	0xcbdf1191, // wheat
	0x229c028a, // wheat1
	0x229c028b, // wheat2
	0x229c028c, // wheat3
	0x229c028d, // wheat4
	0xcbdf6a31, // white
	0x66714d98, // whitesmoke
	0x14d39b7c, // yellow
	0xf057568d, // yellow1
	0xf057568e, // yellow2
	0xf057568f, // yellow3
	0xf0575690, // yellow4
	0xe3380c75, // yellowgreen
	0x32808826, // aliceblue
	0xd78e5de4, // blanchedalmond
	0x9fe53cb3, // blueviolet
	0x6d9ac541, // cadetblue
	0xbcabffb9, // cornflowerblue
	0x4ba1e9b, // crimson
	0x2ef8c3d2, // darkolivegreen
	0xf5dba874, // darkseagreen
	0x81c1c39b, // darkslateblue
	0x81dfe8de, // darkslategray
	0x1bde1805, // deepskyblue
	0xba4ea08d, // dodgerblue
	0xa62bc3c6, // ghostwhite
	0x89a14405, // greenyellow
	0xd1d52955, // hotpink
	0x373ccdde, // indianred
	0x6ad6c70a, // indigo
	0x42d48277, // lavenderblush
	0x9740586a, // lightgoldenrodyellow
	0x349299f2, // lightseagreen
	0xef0d8d9f, // lightskyblue
	0x640ac5cc, // lightslategray
	0x854f23d5, // lightsteelblue
	0x66e38e53, // mediumseagreen
	0xbd025032, // mediumslateblue
	0x5e2144a5, // mediumspringgreen
	0x85252e47, // mediumvioletred
	0x758cb5c4, // midnightblue
	0x14850e60, // olivedrab
	0x251caea0, // palevioletred
};

static UBYTE __far color_rgb[ 2028 ] = {
	0,255,255,
	0,0,128,
	128,128,0,
	255,0,255,
	192,192,192,
	0,128,128,
	0,255,0,
	240,248,255,
	250,235,215,
	255,239,219,
	238,223,204,
	205,192,176,
	139,131,120,
	127,255,212,
	127,255,212,
	118,238,198,
	102,205,170,
	69,139,116,
	240,255,255,
	240,255,255,
	224,238,238,
	193,205,205,
	131,139,139,
	245,245,220,
	255,228,196,
	255,228,196,
	238,213,183,
	205,183,158,
	139,125,107,
	0,0,0,
	255,235,205,
	0,0,255,
	0,0,255,
	0,0,238,
	0,0,205,
	0,0,139,
	165,42,42,
	255,64,64,
	238,59,59,
	205,51,51,
	139,35,35,
	222,184,135,
	255,211,155,
	238,197,145,
	205,170,125,
	139,115,85,
	95,158,160,
	152,245,255,
	142,229,238,
	122,197,205,
	83,134,139,
	127,255,0,
	127,255,0,
	118,238,0,
	102,205,0,
	69,139,0,
	210,105,30,
	255,127,36,
	238,118,33,
	205,102,29,
	139,69,19,
	255,127,80,
	255,114,86,
	238,106,80,
	205,91,69,
	139,62,47,
	100,149,237,
	255,248,220,
	255,248,220,
	238,232,205,
	205,200,177,
	139,136,120,
	0,255,255,
	0,255,255,
	0,238,238,
	0,205,205,
	0,139,139,
	0,0,139,
	0,139,139,
	184,134,11,
	0,100,0,
	169,169,169,
	169,169,169,
	189,183,107,
	139,0,139,
	85,107,47,
	255,140,0,
	153,50,204,
	139,0,0,
	233,150,122,
	143,188,143,
	72,61,139,
	0,206,209,
	148,0,211,
	255,185,15,
	238,173,14,
	205,149,12,
	139,101,8,
	202,255,112,
	188,238,104,
	162,205,90,
	110,139,61,
	255,127,0,
	238,118,0,
	205,102,0,
	139,69,0,
	191,62,255,
	178,58,238,
	154,50,205,
	104,34,139,
	193,255,193,
	180,238,180,
	155,205,155,
	105,139,105,
	255,20,147,
	0,191,255,
	255,20,147,
	238,18,137,
	205,16,118,
	139,10,80,
	0,191,255,
	0,178,238,
	0,154,205,
	0,104,139,
	105,105,105,
	105,105,105,
	30,144,255,
	30,144,255,
	28,134,238,
	24,116,205,
	16,78,139,
	178,34,34,
	255,48,48,
	238,44,44,
	205,38,38,
	139,26,26,
	255,250,240,
	34,139,34,
	220,220,220,
	248,248,255,
	255,215,0,
	255,215,0,
	238,201,0,
	205,173,0,
	139,117,0,
	218,165,32,
	255,193,37,
	238,180,34,
	205,155,29,
	139,105,20,
	0,128,0,
	0,255,0,
	0,238,0,
	0,205,0,
	0,139,0,
	190,190,190,
	0,0,0,
	3,3,3,
	5,5,5,
	8,8,8,
	10,10,10,
	13,13,13,
	15,15,15,
	18,18,18,
	20,20,20,
	23,23,23,
	26,26,26,
	28,28,28,
	31,31,31,
	33,33,33,
	36,36,36,
	38,38,38,
	41,41,41,
	43,43,43,
	46,46,46,
	48,48,48,
	51,51,51,
	54,54,54,
	56,56,56,
	59,59,59,
	61,61,61,
	64,64,64,
	66,66,66,
	69,69,69,
	71,71,71,
	74,74,74,
	77,77,77,
	79,79,79,
	82,82,82,
	84,84,84,
	87,87,87,
	89,89,89,
	92,92,92,
	94,94,94,
	97,97,97,
	99,99,99,
	102,102,102,
	105,105,105,
	107,107,107,
	110,110,110,
	112,112,112,
	115,115,115,
	117,117,117,
	120,120,120,
	122,122,122,
	125,125,125,
	127,127,127,
	130,130,130,
	133,133,133,
	135,135,135,
	138,138,138,
	140,140,140,
	143,143,143,
	145,145,145,
	148,148,148,
	150,150,150,
	153,153,153,
	156,156,156,
	158,158,158,
	161,161,161,
	163,163,163,
	166,166,166,
	168,168,168,
	171,171,171,
	173,173,173,
	176,176,176,
	179,179,179,
	181,181,181,
	184,184,184,
	186,186,186,
	189,189,189,
	191,191,191,
	194,194,194,
	196,196,196,
	199,199,199,
	201,201,201,
	204,204,204,
	207,207,207,
	209,209,209,
	212,212,212,
	214,214,214,
	217,217,217,
	219,219,219,
	222,222,222,
	224,224,224,
	227,227,227,
	229,229,229,
	232,232,232,
	235,235,235,
	237,237,237,
	240,240,240,
	242,242,242,
	245,245,245,
	247,247,247,
	250,250,250,
	252,252,252,
	255,255,255,
	128,128,128,
	0,0,0,
	3,3,3,
	5,5,5,
	8,8,8,
	10,10,10,
	13,13,13,
	15,15,15,
	18,18,18,
	20,20,20,
	23,23,23,
	26,26,26,
	28,28,28,
	31,31,31,
	33,33,33,
	36,36,36,
	38,38,38,
	41,41,41,
	43,43,43,
	46,46,46,
	48,48,48,
	51,51,51,
	54,54,54,
	56,56,56,
	59,59,59,
	61,61,61,
	64,64,64,
	66,66,66,
	69,69,69,
	71,71,71,
	74,74,74,
	77,77,77,
	79,79,79,
	82,82,82,
	84,84,84,
	87,87,87,
	89,89,89,
	92,92,92,
	94,94,94,
	97,97,97,
	99,99,99,
	102,102,102,
	105,105,105,
	107,107,107,
	110,110,110,
	112,112,112,
	115,115,115,
	117,117,117,
	120,120,120,
	122,122,122,
	125,125,125,
	127,127,127,
	130,130,130,
	133,133,133,
	135,135,135,
	138,138,138,
	140,140,140,
	143,143,143,
	145,145,145,
	148,148,148,
	150,150,150,
	153,153,153,
	156,156,156,
	158,158,158,
	161,161,161,
	163,163,163,
	166,166,166,
	168,168,168,
	171,171,171,
	173,173,173,
	176,176,176,
	179,179,179,
	181,181,181,
	184,184,184,
	186,186,186,
	189,189,189,
	191,191,191,
	194,194,194,
	196,196,196,
	199,199,199,
	201,201,201,
	204,204,204,
	207,207,207,
	209,209,209,
	212,212,212,
	214,214,214,
	217,217,217,
	219,219,219,
	222,222,222,
	224,224,224,
	227,227,227,
	229,229,229,
	232,232,232,
	235,235,235,
	237,237,237,
	240,240,240,
	242,242,242,
	245,245,245,
	247,247,247,
	250,250,250,
	252,252,252,
	255,255,255,
	240,255,240,
	240,255,240,
	224,238,224,
	193,205,193,
	131,139,131,
	255,105,180,
	255,110,180,
	238,106,167,
	205,96,144,
	139,58,98,
	205,92,92,
	255,106,106,
	238,99,99,
	205,85,85,
	139,58,58,
	255,255,240,
	255,255,240,
	238,238,224,
	205,205,193,
	139,139,131,
	240,230,140,
	255,246,143,
	238,230,133,
	205,198,115,
	139,134,78,
	230,230,250,
	255,240,245,
	238,224,229,
	205,193,197,
	139,131,134,
	124,252,0,
	255,250,205,
	255,250,205,
	238,233,191,
	205,201,165,
	139,137,112,
	173,216,230,
	240,128,128,
	224,255,255,
	238,221,130,
	144,238,144,
	211,211,211,
	211,211,211,
	255,182,193,
	255,160,122,
	32,178,170,
	135,206,250,
	132,112,255,
	176,196,222,
	255,255,224,
	191,239,255,
	178,223,238,
	154,192,205,
	104,131,139,
	224,255,255,
	209,238,238,
	180,205,205,
	122,139,139,
	255,236,139,
	238,220,130,
	205,190,112,
	139,129,76,
	255,174,185,
	238,162,173,
	205,140,149,
	139,95,101,
	255,160,122,
	238,149,114,
	205,129,98,
	139,87,66,
	176,226,255,
	164,211,238,
	141,182,205,
	96,123,139,
	202,225,255,
	188,210,238,
	162,181,205,
	110,123,139,
	255,255,224,
	238,238,209,
	205,205,180,
	139,139,122,
	50,205,50,
	250,240,230,
	255,0,255,
	255,0,255,
	238,0,238,
	205,0,205,
	139,0,139,
	128,0,0,
	255,52,179,
	238,48,167,
	205,41,144,
	139,28,98,
	102,205,170,
	0,0,205,
	186,85,211,
	147,112,219,
	60,179,113,
	123,104,238,
	0,250,154,
	72,209,204,
	199,21,133,
	224,102,255,
	209,95,238,
	180,82,205,
	122,55,139,
	171,130,255,
	159,121,238,
	137,104,205,
	93,71,139,
	25,25,112,
	245,255,250,
	255,228,225,
	255,228,225,
	238,213,210,
	205,183,181,
	139,125,123,
	255,228,181,
	255,222,173,
	255,222,173,
	238,207,161,
	205,179,139,
	139,121,94,
	0,0,128,
	253,245,230,
	192,255,62,
	179,238,58,
	154,205,50,
	105,139,34,
	255,165,0,
	255,69,0,
	255,165,0,
	238,154,0,
	205,133,0,
	139,90,0,
	255,69,0,
	238,64,0,
	205,55,0,
	139,37,0,
	218,112,214,
	255,131,250,
	238,122,233,
	205,105,201,
	139,71,137,
	238,232,170,
	152,251,152,
	175,238,238,
	219,112,147,
	154,255,154,
	144,238,144,
	124,205,124,
	84,139,84,
	187,255,255,
	174,238,238,
	150,205,205,
	102,139,139,
	255,130,171,
	238,121,159,
	205,104,137,
	139,71,93,
	255,239,213,
	255,218,185,
	255,218,185,
	238,203,173,
	205,175,149,
	139,119,101,
	205,133,63,
	255,192,203,
	255,181,197,
	238,169,184,
	205,145,158,
	139,99,108,
	221,160,221,
	255,187,255,
	238,174,238,
	205,150,205,
	139,102,139,
	176,224,230,
	128,0,128,
	155,48,255,
	145,44,238,
	125,38,205,
	85,26,139,
	255,0,0,
	255,0,0,
	238,0,0,
	205,0,0,
	139,0,0,
	188,143,143,
	255,193,193,
	238,180,180,
	205,155,155,
	139,105,105,
	65,105,225,
	72,118,255,
	67,110,238,
	58,95,205,
	39,64,139,
	139,69,19,
	250,128,114,
	255,140,105,
	238,130,98,
	205,112,84,
	139,76,57,
	244,164,96,
	46,139,87,
	84,255,159,
	78,238,148,
	67,205,128,
	46,139,87,
	255,245,238,
	255,245,238,
	238,229,222,
	205,197,191,
	139,134,130,
	160,82,45,
	255,130,71,
	238,121,66,
	205,104,57,
	139,71,38,
	135,206,235,
	135,206,255,
	126,192,238,
	108,166,205,
	74,112,139,
	106,90,205,
	112,128,144,
	112,128,144,
	131,111,255,
	122,103,238,
	105,89,205,
	71,60,139,
	255,250,250,
	255,250,250,
	238,233,233,
	205,201,201,
	139,137,137,
	0,255,127,
	0,255,127,
	0,238,118,
	0,205,102,
	0,139,69,
	70,130,180,
	99,184,255,
	92,172,238,
	79,148,205,
	54,100,139,
	210,180,140,
	255,165,79,
	238,154,73,
	205,133,63,
	139,90,43,
	216,191,216,
	255,225,255,
	238,210,238,
	205,181,205,
	139,123,139,
	255,99,71,
	255,99,71,
	238,92,66,
	205,79,57,
	139,54,38,
	64,224,208,
	0,245,255,
	0,229,238,
	0,197,205,
	0,134,139,
	238,130,238,
	208,32,144,
	255,62,150,
	238,58,140,
	205,50,120,
	139,34,82,
	245,222,179,
	255,231,186,
	238,216,174,
	205,186,150,
	139,126,102,
	255,255,255,
	245,245,245,
	255,255,0,
	255,255,0,
	238,238,0,
	205,205,0,
	139,139,0,
	154,205,50,
	240,248,255,
	255,235,205,
	138,43,226,
	95,158,160,
	100,149,237,
	220,20,60,
	85,107,47,
	143,188,143,
	72,61,139,
	47,79,79,
	0,191,255,
	30,144,255,
	248,248,255,
	173,255,47,
	255,105,180,
	205,92,92,
	75,0,130,
	255,240,245,
	250,250,210,
	32,178,170,
	135,206,250,
	119,136,153,
	176,196,222,
	60,179,113,
	123,104,238,
	0,250,154,
	199,21,133,
	25,25,112,
	107,142,35,
	219,112,147,
};

unsigned long colhash( char *name )
{
	unsigned long hv = 0;

	while( *name )
	{
		if( !isspace( *name ) )
		{
			hv = hv * 73;
			hv += tolower( *name );
		}
		name++;
	}
	return( hv );
}

int findrgbname( char *name, ULONG *r, ULONG *g, ULONG *b )
{
	ULONG hv = colhash( name );
	int c;

	for( c = 0; c < NUMCOLS; c++ )
	{
		if( color_hash[ c ] == hv )
		{
			UBYTE *cp = &color_rgb[ c * 3 ];
			*r = *cp++;
			*g = *cp++;
			*b = *cp;
			return( TRUE );
		}
	}
	return( FALSE );
}

ULONG colspec2rgb24( char *cs )
{
	ULONG v = 0;
	ULONG r, g, b;
	char buff[ 8 ];
	int c;

	if( findrgbname( cs, &r, &g, &b ) )
	{
		return( MAKE_ID( 0, r, g, b ) );
	}

	while( *cs == '#' )
	{
		cs++;
	}

	memcpy( buff, cs, 6 );
	buff[ 6 ] = 0;
	for( c = 0; c < 6; c++ )
	{
		if( ( buff[ c ] >= 'a' && buff[ c ] <= 'f' )
		 || ( buff[ c ] >= 'A' && buff[ c ] <= 'F' )
		 || ( buff[ c ] >= '0' && buff[ c ] <= '9' )
		)
		continue;
		buff[ c ] = '0';
	}

	stch_l( buff, (LONG*)&v );
	return( v );
}

ULONG muipenspec2rgb24( APTR obj, char *penspec )
{
	ULONG rgb[ 3 ];

	if( *penspec == 's' || *penspec == 'm' || *penspec == 'p' )
	{
		ULONG p = MUI_ObtainPen( muiRenderInfo( obj ), (APTR)penspec, 0 );
#ifndef MBX
//TOFIX!! I've ifdefed it because it wasn't compiling, I'm not sure what this whole
//function is up to, and there was no-one about to ask
		GetRGB32( _screen( obj )->ViewPort.ColorMap, p, 1, rgb );
#endif
		MUI_ReleasePen( muiRenderInfo( obj ), p );
	}
	else
	{
		if( *penspec == 'r' )
			penspec++;

		penspec += stch_l( penspec, (long*)&rgb[ 0 ] );
		penspec++;  //skip comma
		penspec += stch_l( penspec, (long*)&rgb[ 1 ] );
		penspec++;
		penspec += stch_l( penspec, (long*)&rgb[ 2 ] );
	}

	rgb[ 0 ] >>= 24;
	rgb[ 1 ] >>= 24;
	rgb[ 2 ] >>= 24;

	return( rgb[ 0 ] << 16 | rgb[ 1 ] << 8 | rgb[ 2 ] );
}
