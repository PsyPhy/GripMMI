#ifndef	_SGIDISPLAY_

#include "Displays.h"
#include "Graphics.h"

/***************************************************************************/

void	SgiInit( Display display );
void	SgiSwap( Display display );
void	SgiClose( Display display );
void	SgiHardcopy ( Display display );
void	SgiHardcopyPict ( Display display );
void	*SgiSelectFont( char *font_name, int point_size );

int		SgiInput( Display display, float *x, float *y );
void	SgiPoint ( Display display, float x, float y);
void	SgiLine	( Display display, float x1, float y1, float x2, float y2);
void	SgiMoveTo ( Display display, float x1, float y1);
void	SgiLineTo	( Display display, float x2, float y2 );
void	SgiText	( Display display, char *string, 
				  float x, float y, double dir );
float	SgiTextWidth ( Display display, char *string );
float	SgiTextHeight ( Display display, char *string );
void	SgiRectangle	( Display display, 
						  float x1, float y1, float x2, float y2);
void	SgiFilledRectangle ( Display display, 
							 float x1, float y1, float x2, float y2);
void	SgiCircle	(Display display, float x, float y, float radius );
void	SgiFilledCircle	(Display display, float x, float y, float radius );
void	SgiEraseRectangle ( Display display, 
							float x1, float y1, float x2, float y2);
void	SgiErase ( Display display);
void	SgiAlu	( Display display, int alu);
void	SgiLineStyle ( Display display, int style );
void	SgiLinePattern	( Display display, int pattern );
void	SgiColor ( Display display, int color);
void	SgiPenSize ( Display display, float size);
void	SgiStartPolygon ( Display display );
void	SgiAddVertex ( Display display, float x, float y );
void	SgiOutlinePolygon ( Display display );
void	SgiFillPolygon ( Display display );

#define SGI_MAX_POLY_POINTS 1024

typedef struct {

  char	*name;
  long	window;
  struct { 
	float x; 
	float y;
  } vertex[SGI_MAX_POLY_POINTS];
  int   vertex_count;
  float linewidth;
  
} SgiParams;

typedef unsigned long RGBColor;

/* These values are set for the PowerBook screen. */

#define SGI_DISPLAY_TOP		40
#define SGI_DISPLAY_LEFT	30
#define	SGI_DISPLAY_WIDTH	488
#define SGI_DISPLAY_HEIGHT	190

#define _SGIDISPLAY_
#endif
