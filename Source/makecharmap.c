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


#include <stdio.h>

int main( void )
{
	int c;
	char charmap[ 256 ];

	printf( "static char charmap[ 256 ] = { " );

	for( c = 0; c < 256; c++ )
		charmap[ c ] = c;

	charmap[ 128 ] = 'E';   // Euro
	charmap[ 130 ] = ',';
	charmap[ 131 ] = 'f';
	charmap[ 132 ] = '"';
	charmap[ 133 ] = '.';
	charmap[ 134 ] = '+';
	charmap[ 135 ] = '#';
	charmap[ 136 ] = '^';
	charmap[ 137 ] = 'Ø';
	charmap[ 138 ] = '(';
	charmap[ 139 ] = 'S';
	charmap[ 140 ] = 'Ø';
	charmap[ 142 ] = 'Z';
	charmap[ 145 ] = '`';
	charmap[ 146 ] = '\'';
	charmap[ 147 ] = '\"';
	charmap[ 148 ] = '\"';
	charmap[ 149 ] = '·';
	charmap[ 150 ] = '-';
	charmap[ 151 ] = '-';
	charmap[ 152 ] = '~';
	charmap[ 153 ] = '®';
	charmap[ 154 ] = 's';
	charmap[ 155 ] = ')';
	charmap[ 156 ] = 'ø';
	charmap[ 158 ] = 'z';
	charmap[ 159 ] = 'Y';

	charmap[ 27 ] = 32; // Exploit protection...

	for( c = 0; c < 256; c++ )
	{
		printf( "%ld,", charmap[ c ] );
	}
	printf( "};\n" );
	return 0;
}
