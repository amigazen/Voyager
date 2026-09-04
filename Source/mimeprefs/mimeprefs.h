#include "gst.h"
#include "vatstatic.h"

struct mimeinfo {
	char type[ 256 ];
	char ext[ 256 ];
	char app[ 256 ];
	char dir[ 256 ];
	int use_stream, use_internal;
	int use_classdir;
	int act, basetype;
};

extern APTR lv_mime;

// loadvprefs.c
int importvprefs( char *path );
int importibprefs( char *path );

