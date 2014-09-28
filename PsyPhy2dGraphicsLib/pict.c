/*
 * "$Id$"
 *
 *   Output  routines  for  PSGL,  an  SGI Graphics Library (GL) feedback
 *   toolkit for generating Adobe Postscript (tm) output.
 *
 *   This file is part of PSGL.
 *
 *   PSGL  is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY.  No author or distributor  accepts  responsibility  to
 *   anyone for the consequences of using it or for whether it serves any
 *   particular purpose or works at all, unless s/he says so in writing.
 *
 *   PSGL  is PUBLIC DOMAIN.  This means you can make copies, modify, and
 *   redistribute PSGL freely.  PSGL is the collaborative effort  of  the
 *   following individuals:
 *
 *     Seth J. Teller [seth@cs.princeton.edu] March 1991
 *         - Original author.
 *     Loris Renggli [renggli@masgl.epfl.ch] May 1992
 *         - Colormap mode support.
 *     Stuart Priest [priest@yorvic.york.ac.uk] March 1993
 *         - Sphere and callobj support.
 *     Craig Kolb [cek@cs.princeton.edu] December 1993
 *         - Runtime machine detection,
 *         - More colormap support,
 *         - Rudimentary font-manager support.
 *     Mike Sweet [msweet@ctr.nawcad.navy.mil] March 1994
 *         - Munged all the mods into one build,
 *         - Made INST-type distribution image,
 *         - Changed local GL function names to psgl_xxxx() form,
 *         - Added color shading and feedback buffer sizing support.
 *     Joe McIntyre [jam@ccr.jussieu.fr] November 1995
 *         - Added suppression of repeated polygons.
 *         - Added support for European A4 paper.
 *         - Added line patterns in postscript output.
 *         - Added line widths in postscript output.
 *         - Added support for Letter Extra paper.
 *         - Added support for PICT file format output.
 *
 * Contents:
 *
 *
 * Revision History:
 *
 *   $Log$
 *
 */

/*
 * Include necessary headers.
 */

#include <time.h>
#include "private.h"

#include <stdio.h>
#include <math.h>

typedef struct {
  short top;
  short left;
  short bottom;
  short right;
} RECT;

typedef struct {
  short size;
  short top;
  short left;
  short bottom;
  short right;
} Region;

typedef struct {
  short y;
  short x;
} Point;

typedef struct {
  char y;
  char x;
} Delta;

typedef struct {
  unsigned short red;
  unsigned short green;
  unsigned short blue;
} RGBColor;

typedef unsigned short Opcode;

#define HEADER 0x0c00

#define NOP 0x0000
#define CLIP 0x0001
#define PENSIZE 0x0007
#define FOREGROUND 0x000e
#define BACKGROUND 0x000f

#define RGBFOREGROUND 0x001a
#define RGBBACKGROUND 0x001b

#define TEXTFONT 0x0003
#define TEXTSIZE 0x000d
#define LONGTEXT 0x0028

#define LINE 0x0020
#define FRAMERECTANGLE 0x0030
#define PAINTRECTANGLE 0x0031

#define FRAMESAMERECTANGLE 0x0038
#define PAINTSAMERECTANGLE 0x0039

#define FRAMEPOLYGON 0x0070
#define PAINTPOLYGON 0x0071
#define FILLPOLYGON 0x007c

#define FRAMESAMEPOLYGON 0x0078
#define PAINTSAMEPOLYGON 0x0079
#define FILLSAMEPOLYGON 0x007c

#define SHORTLINE 0x0022
#define SHORTLINEFROM 0x0023

#define LONGCOMMENT 0x00a1
#define SHORTCOMMENT 0x00a0

#define ENDOFPICTURE 0x00ff

Opcode Nop = NOP;
Opcode Header = HEADER;
unsigned long Version = 0x001102ffL;

Opcode EndOfPicture = ENDOFPICTURE;

Opcode Line = LINE;

Opcode FrameRectangle = FRAMERECTANGLE;
Opcode PaintRectangle = PAINTRECTANGLE;

Opcode FramePolygon = FRAMEPOLYGON;
Opcode PaintPolygon = PAINTPOLYGON;
Opcode FillPolygon = FILLPOLYGON;

Opcode FrameSamePolygon = FRAMESAMEPOLYGON;
Opcode PaintSamePolygon = PAINTSAMEPOLYGON;
Opcode FillSamePolygon = FILLSAMEPOLYGON;

Opcode Foreground = FOREGROUND;
Opcode Background = BACKGROUND;

Opcode RGBForeground = RGBFOREGROUND;
Opcode RGBBackground = RGBBACKGROUND;

Opcode Clip = CLIP;
Opcode PenSize = PENSIZE;
Opcode ShortLine = SHORTLINE;
Opcode ShortLineFrom = SHORTLINEFROM;

Opcode LongComment = LONGCOMMENT;
Opcode ShortComment = LONGCOMMENT;

Opcode LongText = LONGTEXT;

/* Selects the Helvetica Font. */
char *Helvetica = "\000\054\000\014\000\025\011Helvetica\000\003\000\025";
Opcode TextSize = TEXTSIZE;
short default_font_size = 12;

#define output( x ) fwrite( &x, sizeof( x ), 1, psgl_file )
#define outstring( x ) fwrite( x, strlen( x ), 1, psgl_file )

/*****************************************************************************/

static int x_range, y_range;
static int x_offset, y_offset;

/*****************************************************************************/

char *psgl_open_pict ( char *filename ) {
  
  fprintf( stderr, "Output in PICT format. \n" );
  psgl_output_pict = TRUE;
  return( psgl_open( filename ) );

}

/*****************************************************************************/

void psgl_emit_pict_prologue ( void ) {

  char header[512];
  short left;
  short top;
  short right;
  short bottom; 

  Region clip_region;
  RGBColor color;
  RECT  rect;
  long size;


  unsigned short PictureSize;
  RECT PictureFrame;

  Point pen_size = { 1, 1 };

  /* This is what I found in a ClarisDraw file. */
  long x_resolution = 0xffff000;
  long y_resolution = 0xffff0000;

  /* Header is an unused 512 byte buffer. */
  output( header );

  /* Picture size is probably the size in bytes, but we don't know it yet. */
  PictureSize = 1302;
  output( PictureSize );

  /* Calculate image size. */
  x_range = ceil( psgl_xmax ) - floor( psgl_xmin );
  y_range = ceil( psgl_ymax ) - floor( psgl_ymin );
  x_offset = floor( psgl_xmin );
  y_offset = floor( psgl_ymin );

  PictureFrame.left = -1;
  PictureFrame.top = -1;
  PictureFrame.right = x_range + 2;
  PictureFrame.bottom = y_range + 2;
  output( PictureFrame );

  /* Version is a known byte code. */
  output( Version );

  /* Identify the second header. */
  output( Header );

  /* Version 2 files have -1 here. */
  size = -1;
  output( size );
  
  output( x_resolution );
  output( y_resolution );

  /*
  left = 0;
  top = 0;
  */
  right = PictureFrame.right;
  bottom = PictureFrame.bottom;
  left = PictureFrame.left;
  top = PictureFrame.top;
  output( right );
  output( bottom );
  output( top );
  output( left );

  output( Nop );
  output( Nop );

  clip_region.size = 10;
  clip_region.left = -1;
  clip_region.top = -1;
  clip_region.right = PictureFrame.right;
  clip_region.bottom = PictureFrame.bottom;
  output( Clip );
  output( clip_region );

  output( PenSize );
  output( pen_size );

  /* Select a font. */
  /*
	fwrite( Helvetica, sizeof( Helvetica ) - 1, 1, psgl_file );
	*/

  /* Set default font size to 12 points. */
  output( TextSize );
  output( default_font_size );

  fflush( psgl_file );

}

/*****************************************************************************/

void
psgl_emit_cache_pict (void)
{
  int          type,
               i,
               k;
  CACHE_ITEM   *item;
  CACHE_POINT  *vptr; 
  static float current_rgb[3];
  static float current_fontsize;

  int   duplicate;
  int   current_length = -1;
  float current_x[256], current_y[256];

  Point    point;
  Delta    delta;
  RECT     rect;
  RGBColor color;
  unsigned short length;
  unsigned short text_size;
  unsigned char  clength;

  current_fontsize = -1.0;

  for (k = 0; k < psgl_cache_index; k ++)
  {
    item = psgl_cache + k;
    type = item->type;

    if (k == 0 ||
        item->r != current_rgb[0] ||
        item->g != current_rgb[1] ||
        item->b != current_rgb[2])
    {
      current_rgb[0] = item->r;
      current_rgb[1] = item->g;
      current_rgb[2] = item->b;

	  if ( item->r == 0.0 &&
		   item->g == 0.0 &&
		   item->b == 0.0 ) {
		color.red = color.green = color.blue = 0xffff;
	  }
	  else {
		color.red = item->r * 0xffff;
		color.green = item->g * 0xffff;
		color.blue = item->b * 0xffff;
		
	  }
	  output( RGBForeground );
	  output( color );
	}


#if 1

    switch (type) {

	case CACHED_POINT :
	  vptr = (CACHE_POINT *)item->data;
	  for (i = 0; i < item->length; i ++) {

		point.x = vptr[i].px - x_offset;
		point.y = y_range - ( vptr[i].py - y_offset);
		delta.x = 0;
		delta.y = 0;
		output( ShortLine );
		output( point );
		output( delta );

	  }
	  break;

	case CACHED_VECTOR :
	case CACHED_VECTORLOOP :
	case CACHED_POLYGON :

	  vptr = (CACHE_POINT *)item->data;

	  duplicate = TRUE;
	  if ( current_length != item->length ) duplicate = FALSE;
	  for ( i = 0; i < item->length && duplicate; i++ ) {
		if ( vptr[i].px != current_x[i] || vptr[i].py != current_y[i] ) {
		  duplicate = FALSE;
		}
	  }
          
	  if ( !duplicate ) {
		
		current_length = item->length;
		point.x = point.y = item->line_width;
		output( PenSize );
		output( point );
		
		if ( current_length == 2 ) {
		  output( Line );
		  point.x = vptr[0].px - x_offset;
		  point.y = y_range - ( vptr[0].py - y_offset );
		  output( point );
		  point.x = vptr[1].px - x_offset;
		  point.y = y_range - ( vptr[1].py - y_offset );
		  output( point );
		}
		else {

		  if (item->type == CACHED_POLYGON) output( PaintPolygon );
		  else output( FramePolygon );

		  length = 10 + item->length * sizeof( Point );
		  if (type == CACHED_VECTORLOOP || type == CACHED_POLYGON) {
			length += sizeof( Point );
		  }
		  output( length );

		  rect.left = item->xmin - x_offset - 1;
		  rect.right = item->xmax - x_offset + 1;
		  rect.top = y_range - ( item->ymax - y_offset ) - 1;
		  rect.bottom = y_range - ( item->ymin - y_offset ) + 1;
		  output( rect );

		  for (i = 0; i < item->length; i++) {
			point.x = vptr[i].px - x_offset;
			point.y = y_range - ( vptr[i].py - y_offset );
			output( point );
			current_x[i] = vptr[i].px;
			current_y[i] = vptr[i].py;
		  }

		  if (type == CACHED_VECTORLOOP || type == CACHED_POLYGON) {
			point.x = vptr[0].px - x_offset;
			point.y = y_range - ( vptr[0].py - y_offset );
			output( point );
		  }


		}
	  }
	  break;

	case CACHED_STRING :

	  /*
	   * convert char height (in pixels) to postscript height (in points)
	   */

	  text_size = item->h;
	  output( TextSize );
	  output( text_size );

	  point.x = item->x - x_offset;
	  point.y = y_range - (item->y - y_offset);

	  output( LongText );
	  output( point );
	  clength = strlen( item->data );
	  output( clength );
	  outstring( item->data );
	  /* Make sure we're left on a word boundary. */
	  if ( !( clength % 2 ) ) {
		output( clength );
	  }

	  break;
    }
#endif
  }
}


/*****************************************************************************/

void psgl_emit_pict_epilogue ( void ) {

  output( EndOfPicture );

}

#ifdef MAKE_MAIN

main () {

  RECT rect;
  Point point;
  RGBColor color;

  unsigned char clength;
  char message[] = "Message";

  psgl_emit_pict_prologue();


  rect.left = 0;
  rect.right = 50;
  rect.top = 0;
  rect.bottom = 60;

  output( PaintRectangle );
  output( rect );

  color.red = 0xffff;
  color.green = 0x7fff;
  color.blue = 0x0000;
  output( RGBForeground );
  output( color );


  rect.left = 200;
  rect.right = 250;
  rect.top = 300;
  rect.bottom = 360;

  output( FrameRectangle );
  output( rect );

  point.x = 400;
  point.y = 300;
  clength = strlen( message );

  output( LongText );
  output( point );
  output( clength );
  outstring( message );
  
  psgl_emit_pict_epilogue();
}
#endif


