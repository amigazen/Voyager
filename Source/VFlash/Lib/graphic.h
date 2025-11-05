/////////////////////////////////////////////////////////////
// Flash Plugin and Player
// Copyright (C) 1998 Olivier Debon
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// 
///////////////////////////////////////////////////////////////
// $Id: graphic.h,v 1.1.1.1 2002/10/07 12:52:16 zapek Exp $
#ifndef _GRAPHIC_H_
#define _GRAPHIC_H_

#define ALPHA_OPAQUE 255

enum FillType {
	f_Solid          = 0x00,
	f_LinearGradient = 0x10,
	f_RadialGradient = 0x12,
	f_TiledBitmap    = 0x40,
	f_clippedBitmap  = 0x41,
	f_None		 = 0x80
};

struct Gradient {
	int		 nbGradients;
	unsigned char	 ratio[8];
	Color		 color[8];
	// For rendering
	Color		*ramp;
	Matrix		 imat;
	int has_alpha;
};


struct FillStyleDef {
	FillType	 type;	// See enum FillType
	
	// Solid
	Color		 color;
	
	// Gradient
	Gradient	 gradient;
	
	// Bitmap
	Bitmap		*bitmap;
	Matrix              bitmap_matrix;
	Color               *cmap;
	unsigned char *alpha_table;

	// Gradient or Bitmap
	Matrix		 matrix;

	FillStyleDef() {
		style_size += sizeof(FillStyleDef);
		style_nb++;
	}
};

struct Segment {
	long x1,x2;
	long		 ymax;
	FillStyleDef	*fs[2];	// 0 is left 1 is right
	int		 aa;
	long		 dX;
	long		 X;
	
	struct Segment *next;
	struct Segment *nextValid;
};

/* fractional bits (we don't use twips here... too expensive) */
#define FRAC_BITS    5
#define FRAC         (1 << FRAC_BITS)
#define NB_SEGMENT_MAX (2048*4)
#define SEGFRAC	     8

class GraphicDevice {
	Color			 backgroundColor;
	Color			 foregroundColor;
	int			 targetWidth;
	int 			 targetHeight;
	Rect			 viewPort;
	int			 movieWidth;
	int			 movieHeight;
	int			 zoom;
	unsigned long		 redMask;
	unsigned long		 greenMask;
	unsigned long		 blueMask;
	int			 clipping;
	FlashDisplay		*flashDisplay;
	int			 bgInitialized;

public:
		/* polygon rasterizer */

		Segment **segs;
		int ymin,ymax;
		int height;
		Segment *seg_pool;
		Segment *seg_pool_cur;

		void *scan_line_func_id;
		ScanLineFunc scan_line_func;

	long			 showMore;	// Used for debugging
	Rect			 clip_rect;

protected:
	long	 clip(long &y, long &start, long &end);

public:
	Matrix			*adjust;	// Matrix to fit window (shrink or expand)

	// For Direct Graphics
	unsigned char 		*canvasBuffer;	// A pointer to canvas'memory
	long			 bpl;	// Bytes per line
	long			 bpp;	// Bytes per pixel
	long			 pad;	// Scanline pad in byte

	GraphicDevice(FlashDisplay *fd);
	~GraphicDevice();

	int	 setBackgroundColor(Color);
	void	 setForegroundColor(Color);
	Color	 getBackgroundColor();
	Color	 getForegroundColor();
	void	 setMovieDimension(long width, long height);
	void	 setMovieZoom(int zoom);
	void	 setMovieOffset(long x, long y);
	void	 clearCanvas();
	long	 getWidth();
	long	 getHeight();
	long	 (*allocColor)(Color color);
	Color 	*getColormap(Color *old, long n, Cxform *cxform);
		void     fillLineBitmap(FillStyleDef *f, long y, long start, long end);
	void	 fillLineLG(Gradient *grad, long y, long start, long end);
	void	 fillLineRG(Gradient *grad, long y, long start, long end);
		void     fillLine(FillStyleDef *f, long y, long start, long end);
		void     fillLineAA(FillStyleDef *f, long y, long start, long end);

		void	 drawLine(long x1, long y1, long x2, long y2, long width);
	void 	 drawBox(long x1, long y1, long x2, long y2);

		void     addSegment(long x1, long y1, long x2, long y2,
							FillStyleDef *f0,
							FillStyleDef *f1,
							int aa);
		
		void     drawPolygon(void);

	void	 updateClippingRegion(Rect *);
	void	 setClipping(int);
};

#endif /* _GRAPHIC_H_ */
