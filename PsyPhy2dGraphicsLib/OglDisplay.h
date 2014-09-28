#ifndef	_OGLDISPLAY_

#include "Displays.h"
#include "Graphics.h"
#include "OglDisplayInterface.h"

/***************************************************************************/


#ifdef __cplusplus 
extern "C" {
#endif

Display CreateOglDisplay( void );
DestroyOglDisplays( void );

void	OglInit( Display display );
void	OglActivate( Display display );
void	OglSwap ( Display display );

void	OglClose( Display display );
void	OglHardcopy ( Display display, char *filename );
void	OglHardcopyPict ( Display display );

int		OglInput( Display display, float *x, float *y );
void	OglPoint ( Display display, float x, float y);
void	OglLine	( Display display, float x1, float y1, float x2, float y2);
void	OglMoveTo ( Display display, float x1, float y1);

void	OglStartTrace	( Display display, float x, float y );
void	OglContinueTrace	( Display display, float x, float y );
void	OglEndTrace	( Display display, float x, float y );


void	OglLineTo	( Display display, float x2, float y2 );
void	OglText	( Display display, char *string, 
				  float x, float y, double dir );
float	OglTextWidth ( Display display, char *string );
float	OglTextHeight ( Display display, char *string );
void	OglRectangle	( Display display, 
						  float x1, float y1, float x2, float y2);
void	OglFilledRectangle ( Display display, 
							 float x1, float y1, float x2, float y2);
void	OglCircle	(Display display, float x, float y, float radius );
void	OglFilledCircle	(Display display, float x, float y, float radius );
void	OglEraseRectangle ( Display display, 
							float x1, float y1, float x2, float y2);
void	OglErase ( Display display);
void	OglAlu	( Display display, int alu);
void	OglLineStyle ( Display display, int style );
void	OglLinePattern	( Display display, int pattern );
void	OglColor ( Display display, int color);
void	OglColorRGB ( Display display, float r, float g, float b );
void	OglPenSize ( Display display, float size);
void	OglStartPolygon ( Display display );
void	OglAddVertex ( Display display, float x, float y );
void	OglOutlinePolygon ( Display display );
void	OglFillPolygon ( Display display );

#ifdef __cplusplus 
}
#endif

#define OGL_MAX_POLY_POINTS 255

typedef struct {

  char		*name;
  OglWindow ogl_window;

  struct { 
	float x; 
	float y;
  } vertex[OGL_MAX_POLY_POINTS];
  int   vertex_count;

  float last_x;
  float last_y;

  float ai_scale;
  float ai_offset_x;
  float ai_offset_y;
  FILE  *cpy;
  
} OglParams;

 
typedef unsigned long RGBColor;


#define ToAiX(x) ((x) * params->ai_scale + params->ai_offset_x)
#define ToAiY(y) ((y) * params->ai_scale + params->ai_offset_y)

/* These values are set for the PowerBook screen. */

#define OGL_DISPLAY_TOP		40
#define OGL_DISPLAY_LEFT	30
#define	OGL_DISPLAY_WIDTH	488
#define OGL_DISPLAY_HEIGHT	190

#define _OGLDISPLAY_
#endif
