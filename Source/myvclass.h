/*
**
** $Id: myvclass.h,v 1.32 2000/07/02 04:43:52 sircus Exp $
**
*/

#ifndef MYVCLASS_H
#define MYVCLASS_H

#define GETMYDATA(o,p) p=INST_DATA(OCLASS(o),o)

#define FONTLEADING 0
#define LIINDENT 40

extern struct Library *CyberGfxBase;

/*
 * Select mode
 */
#define SELECTMODE_NONE	  0
#define SELECTMODE_START  1
#define SELECTMODE_ACTIVE 2

struct pennode {
	struct MinNode n;
	ULONG r, g, b;
	ULONG pen;
};

struct form {
	struct MinNode n;
	ULONG id;
	UWORD method;       // POST, GET, ISINDEX
	UWORD enctype;		// TRUE for multipart, FALSE urlencoded
	char *url;          // Form URL
	char *target;
	APTR js_form;
};

struct textobj {
	struct MinNode n;
	LONG y;
	WORD x;
	UWORD len;
	UBYTE markstart, markend; // marked text
	UWORD objid, linenum;
	struct TextFont *fontptr;
	UBYTE style;
	UBYTE tcol;
	char text[ 2 ];
};

struct bgrectobj {
	struct MinNode n;
	LONG x, y, xe, ye;
	ULONG pen;
	APTR bgimg;
	struct BitMap *bgbitmap;
	int bgixs, bgiys;
};

#define MAXFONTSTACK 128
struct fontstack {
	int old_st_font;
	int old_col;
	char old_st_font_face[ 32 ];
};

#define MYV_NUMPENS 6
enum {
	myvp_background,
	myvp_text,
	myvp_link,
	myvp_vlink,
	myvp_shadow,
	myvp_shine
};

struct imageloadnode {
	struct MinNode n;
	struct imgclient *imgclient;
	int ignoreme;
};

struct hashnode {
	struct MinNode n;
	char name[ 80 ];
	ULONG offset;
};

struct Data {
	UWORD pad;
	APTR textobjpool;
	ULONG xsize;
	ULONG ysize;
	int lastystart;
	//ULONG vtop, vleft;
	struct MinList l, bgl, bgl2;
	ULONG pens[ MYV_NUMPENS ];
	int pens_allocated;
	struct MUI_PenSpec pencolors[ 4 ];
	struct nstream *doc;
	ULONG nodrawlock;
	struct BitMap *bgbitmap;
	ULONG bgx, bgy;
	int imagecount;
	struct MinList imagelist;
	struct MinList forms, hashlist, imaplist, penlist;
	UWORD relayouting, setupcount;
	ULONG oldmwidth, oldmheight;
	APTR loadimagepool;
	UWORD hadbg, enabledforms;
	APTR contextmenu_link, contextmenu_image, contextmenu_imagelink;
	ULONG lasturlgot;
	int lasturltnest;
	struct imgclient *bgimg;
	int needbackground;
	int waitforimages;
	int gotpenspec;
	int noinstallclip;
	char title[ 256 ];
	char *framename;
	int iterdepth;          // frame nesting depth
	int printmode;
	int isframeset;         // is a frameset container
	int marginwidth, marginheight;
	int image_reload;
	APTR parentframeset;

	int ihn_active;
	struct MUI_InputHandlerNode ihn; // for refresh
	char *refreshurl;         // for refresh
	int refreshtimeout;             // in seconds

	int docdone;

	int oldtop; // top on last draw

	int istopcontainer;

	int selectmode;
	int selectstart_x, selectstart_y, selectend_x, selectend_y;

	int maxtobjid;

	ULONG lastqual;

	int isloadingimage;

	int is_static_table;

	APTR printwin;

	int whitepen; // for printing

	APTR searchwin;
	ULONG lasts_tobjid;
	int lasts_tindex;

	struct jsop_list *jso; // Javascript context

	int markarea_xs, markarea_ys, markarea_xe, markarea_ye;

	APTR lasthitobj;
	APTR lastjsobject;

	// for document write, push buffer
	struct MinList pushbuffer;

	struct MinList customprops;

	// document client arrays (created "on demand")
	struct MinList domos;
	APTR jsa_forms, jsa_images, jsa_links, jsa_anchors;

	// window size tracking
	LONG last_win_width;
	LONG last_win_height;

	char lasturl[ 256 ];
	struct MUI_EventHandlerNode ehnode;
};

struct pushnode {
	struct MinNode n;
	char str[ 0 ]; // to be extended
};

/*
struct BresenhamInfo
{
	LONG minsize;
	LONG maxsize;
	LONG defsize;
	LONG cursize;
	LONG weight;
	int fixed;
	int semifixed;
};
*/

struct tablecell {
	struct MinNode n;
	int width, height;
	int colspan;
	int rowspan;
	int xp, yp;
	int targetx;
	int row;
	int suggested_width, suggested_height;
	int suggested_width_perc;
	int minimumwidth, maximumwidth;
	int valign, voffset;
	int bgpen;
	APTR bgimg;
};

struct tablecontext {
	struct MinNode n;
	int st_xoffset, yp, xp, targetx, maxx, st_center, st_right;
	int old_stxoffset;
	int st_pright, st_pleft, st_pcenter;
	int resetxposleft, resetxposright;
	int minimumwidth;
	int st_td, st_tr, st_p;
	int st_pre, st_nobr;
	int st_font, currentcol;
	char st_font_face[ 32 ];
	int doingtablelayout;
	ULONG tablestart; // previous one
	struct tablecell *currentcell; // previous one
	int tablerow;
	struct MinList l; // cell list
	struct MinList nl;
	int bordersize;
	struct MinList fol; // frame object list
	int endy, endx;
	int centertable, righttable, tableoffset;
	int captionoffset;
	int tablewidth, tableheight;
	int tablewidthperc;
	int listlevel;
	int cellpadding, cellspacing;
	struct tablecell **cellarray;
	int maxrow, maxcol;
	struct BresenhamInfo *binfo;
	int color_shadow, color_shine;
	int tabminwidth;
	int bgpen;
	int tablestatic, tablestaticwidth;
	ULONG thistablestart;
	int bgrect_x, bgrect_y, bgrect_xe, bgrect_ye;
	struct fontstack fontstack[ MAXFONTSTACK ];
	int fontstackptr;
	int last_tr_bgpen, last_tr_valign, last_tr_align;
	APTR bgimg;
};

struct tablesizeinfo {
	struct MinNode n;
	ULONG tablestart, tableend;
	int xsize, ysize, origwidth, staticmode;
	int minimumwidth;
};

struct formsizeinfo {
	struct MinNode n;
	ULONG offset;
	int xs, ys;
};

struct fontnode {
	struct MinNode n;
	char name[ 32 ];
	struct TextFont *tf;
	UBYTE fontarray[ 257 ];
};


//1
APTR d_alloc( struct Data *data, int size );
APTR d_alloccp( struct Data *data, APTR oo, int size );
APTR d_strdup( struct Data *data, STRPTR s );
void addbgrectobj( struct Data *data, int which, int x, int y, int xe, int ye, int pen );
void addbgrectimage( struct Data *data, int which, int x, int y, int xe, int ye, APTR bgi );

extern struct MUI_CustomClass *myvclass_mcc;


// 2
void processframeset( APTR obj, struct Data *data, char *baseref, char **token, int xp, int yp, int xs, int ys, int *iter, int marginwidth, int marginheight, int imageloadmode, int bordersize );
void dotablayout( struct Data *data, APTR obj, int layoutonly, struct tablecontext *tab, ULONG *xp, ULONG *yp, ULONG *maxx, int targetx, int *endx, int *endy );
void dotabcellwidths( struct Data *data, struct tablecontext *tab );
void newimagemap( struct Data *w, char *name );
void newimaparea( struct Data *data, struct imagemap *im, int type, char *url, char *target, char *coords, char *alttext );
struct imagemap *findimagemap( struct Data *w, char *name );
void setimagemaps( APTR obj, struct Data *data );
void allocpens( APTR o, struct Data *data );
void freepens( APTR o, struct Data *data );
ULONG getpencolname( APTR o, struct Data *data, char *color );
ULONG getpenrgb( APTR o, struct Data *data, ULONG r, ULONG g, ULONG b );
void selectit( APTR obj, struct Data *data );
void put2clip( struct Data *data, APTR obj );

void js_doeventhandlers( APTR refwin, struct jsop_list *jol, APTR obj, int lineno, struct Data *data );

extern struct Hook layouthook;

extern ASM int ptextlen( __reg(a0,STRPTR text), __reg(a1,struct TextFont *tf), __reg(d0, UWORD textlen), __reg(d1, UBYTE softstyles ));
extern ASM int ptextfit( __reg(a0,STRPTR text), __reg(a1,struct TextFont *tf), __reg(d0, UWORD textlen), __reg(d1, UWORD pixelsize), __reg(d2, UBYTE softstyles) );

#endif /* MYVCLASS_H */
