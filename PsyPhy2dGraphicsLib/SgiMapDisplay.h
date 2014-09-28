#ifndef	_SGIDISPLAY_

#include "Displays.h"
#include "Graphics.h"

/***************************************************************************/

void	SgiMapInit( Display display );
void	SgiMapClose( Display display );
void	SgiMapHardcopy ( Display display );
void	SgiMapHardcopyPict ( Display display );

int		SgiMapInput( Display display, float *x, float *y );
void	SgiMapPoint ( Display display, float x, float y);
void	SgiMapLine	( Display display, float x1, float y1, float x2, float y2);
void	SgiMapMoveTo ( Display display, float x1, float y1);
void	SgiMapLineTo	( Display display, float x2, float y2 );
void	SgiMapText	( Display display, char *string, 
				  float x, float y, double dir );
float	SgiMapTextWidth ( Display display, char *string );
float	SgiMapTextHeight ( Display display, char *string );
void	SgiMapRectangle	( Display display, 
						  float x1, float y1, float x2, float y2);
void	SgiMapFilledRectangle ( Display display, 
							 float x1, float y1, float x2, float y2);
void	SgiMapCircle	(Display display, float x, float y, float radius );
void	SgiMapFilledCircle	(Display display, float x, float y, float radius );
void	SgiMapEraseRectangle ( Display display, 
							float x1, float y1, float x2, float y2);
void	SgiMapErase ( Display display);
void	SgiMapAlu	( Display display, int alu);
void	SgiMapLineStyle ( Display display, int style );
void	SgiMapLinePattern	( Display display, int pattern );
void	SgiMapColor ( Display display, int color);
void	SgiMapPenSize ( Display display, float size);
void	SgiMapStartPolygon ( Display display );
void	SgiMapAddVertex ( Display display, float x, float y );
void	SgiMapOutlinePolygon ( Display display );
void	SgiMapFillPolygon ( Display display );

#define SGI_MAX_POLY_POINTS 1024

typedef struct {

  char	*name;
  long	window;
  struct { 
	float x; 
	float y;
  } vertex[SGI_MAX_POLY_POINTS];
  int   vertex_count;
  
} SgiMapParams;


typedef struct {
  short red;
  short green;
  short blue;
} SgiMapColorDef;

typedef unsigned long RGBColor;

extern Display		SgiMapDisplay;

/* These values are set for the PowerBook screen. */

#define SGI_DISPLAY_TOP		40
#define SGI_DISPLAY_LEFT	30
#define	SGI_DISPLAY_WIDTH	488
#define SGI_DISPLAY_HEIGHT	190

#define _SGIDISPLAY_
#endif
