#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "windows.h"

#include <gl/gl.h>
#include "glut.h"
#include "useful.h"
#include "Graphics.h"
#include "Displays.h"
#include "OglDisplay.h"

#include "OglDisplayInterface.h"

local double ogl_font_height = 12;
local double ogl_font_width = 8;

/***************************************************************************/

// Create a static version of an OglDisplay.
OglParams	_ogl_params = {"Ogl 2D Display"};
struct _display	_OglDisplay = {
  "Ogl Display",
    20, 20, 600, 600,
    OglPoint, OglLine, OglMoveTo, OglLineTo,

    OglStartTrace, OglContinueTrace, OglEndTrace,

    OglText, OglTextWidth, OglTextHeight,
    OglRectangle, OglFilledRectangle,
    OglCircle, OglFilledCircle,
    
    OglStartPolygon, 
    OglAddVertex, 
    OglOutlinePolygon, 
    OglFillPolygon,
    
    OglErase, OglEraseRectangle,
    OglLineStyle, OglLinePattern,
    OglColor, OglColorRGB, OglAlu, 
    OglPenSize,
    OglInit, OglActivate, OglSwap, OglClose, OglHardcopy, OglInput,
    SOLID,						/* Line Pattern */
    SET,						/* ALU */
    FOREGROUND,					/* Color */
    FALSE,						/* Black and White */
    3,							/* Symbol Size (radius) */
    -1, -1,						/* Desired Width and Height */
	0, 0,						/* Desired Left and Top */
    NULL, NULL, NO,				/* Redraw cache */
	NULL,						/* Linked list next element */
    &_ogl_params
};
Display		OglDisplay = &_OglDisplay; // Pointer to the static OglDisplay.
Display		_ogl_display_list = NULL;

/***************************************************************************/

void OglDisplayRedraw ( Display display ) {
	DisplayWalkCache( display, display );  
	Swap();
}

/***************************************************************************/

void OglHardcopy ( Display display, char *filename ) {

  register OglParams	*params = (OglParams *) display->parameters;

  params->cpy = fopen( filename, "w" );

  if ( !params->cpy ) {
    fprintf( stderr, "Error opening %s for write.\n", filename );
  }
  else {

    fprintf( stderr, "%s open for write ...", filename );
    fprintf( params->cpy, "%%!PS-Adobe-2.0 EPSF-1.2\n" );
    fprintf( params->cpy, "%%%%Creator: Adobe Illustrator(TM) for Windows, version 4.0\n" );
    fprintf( params->cpy, "%%%%BoundingBox: %.0f %.0f %.0f %.0f\n",
        ToAiX( display->left ), ToAiY( display->bottom ),
        ToAiX( display->right ), ToAiY( display->top ) );

    fprintf( params->cpy, "%%%%TemplateBox:  %.0f %.0f %.0f %.0f\n",
        ToAiX( display->left ), ToAiY( display->bottom ),
        ToAiX( display->right ), ToAiY( display->top ) );

    fprintf( params->cpy, "%%AI3_ColorUsage: Color\n" );
    fprintf( params->cpy, "%%AI3_TemplateBox: 306.5 395.5 306.5 395.5\n" );
    fprintf( params->cpy, "%%AI3_TileBox: 18 23 594 779\n" );
    fprintf( params->cpy, "%%AI3_DocumentPreview: None\n" );
    fprintf( params->cpy, "%%%%PageOrigin:18 23\n" );
    fprintf( params->cpy, "%%%%AI3_PaperRect:-18 779 594 -13\n ");
    fprintf( params->cpy, "%%%%AI3_Margin:18 -18 -18 13\n ");

    fprintf( params->cpy, "%%%%EndComments\n" );
    fprintf( params->cpy, "%%%%BeginProlog\n" );
    fprintf( params->cpy, "%%%%EndProlog\n" );
    fprintf( params->cpy, "%%%%BeginSetup\n" );
    fprintf( params->cpy, "%%%%EndSetup\n" );

    fprintf( params->cpy, "u\n" );

    fprintf( params->cpy, "0 0 0 1 k\n" );
    fprintf( params->cpy, "0 0 0 1 K\n" );

//    fprintf( params->cpy, "/_ArialMT %.0f %.0f 0 0 z\n", 
//      ceil( (double) ogl_font_height * 1.3), ceil( (double) ogl_font_height * 1.3) + 1 );
    fprintf( params->cpy, "/_CourierNewPSMT %.2f %.2f 0 0 z\n", 
      (double) ogl_font_height * 1.15 * params->ai_scale, 
      (double) ogl_font_height * 1.15 * params->ai_scale );
   

    DisplayWalkCache( display, display );
    Swap();

    fprintf( params->cpy, "U\n" );

    fprintf( params->cpy, "%%%%Trailer\n" );
    fclose( params->cpy );

    fprintf( stderr, "OK.\n" );

    params->cpy = 0;

  }
}

void OglHardcopyPict ( Display display ) {}

/***************************************************************************/

Display CreateOglDisplay( void ) {

	OglParams *params;
	Display	  display;

	params = malloc( sizeof( OglParams ) );
	params->ogl_window.hDC = NULL;
	params->ogl_window.hRC = NULL;
	params->ogl_window.hWnd = NULL;
	if ( !params ) {
		MessageBox( NULL, "Error allocating memory for OglParams.", "OglDisplay.c", MB_OK );
		exit( -100 );
	}
	params->name = "Dynamic OglDisplay";
	display = malloc( sizeof( *display ) );
	if ( !display ) {
		MessageBox( NULL, "Error allocating memory for Display.", "OglDisplay.c", MB_OK );
		exit( -101 );
	}
	memcpy( display, OglDisplay, sizeof( *display ) );
	display->parameters = params;
	display->next = _ogl_display_list;
	_ogl_display_list = display;

	return( display );

}

DestroyOglDisplays( void ) {

	Display display = _ogl_display_list;
	Display display_to_kill;

	while ( display ) {
		OglClose( display );
		free( display->parameters );
		display_to_kill = display;
		display = display->next;
		free( display_to_kill );
	}

}

/***************************************************************************/

local Display _ogl_display;

local void _refresh_ogl ( HWND window ) { 

	register OglParams	*params;
	Display  display = _ogl_display_list;
	
	// Check if the static OglDisplay needs to be redrawn.
	params = (OglParams *) OglDisplay->parameters;
	if ( params->ogl_window.hWnd == window ) {
		OglActivate( OglDisplay );
		OglDisplayRedraw( OglDisplay );
	}

	// Now work through the list of the dynamic displays.
	while ( display ) {
		params = (OglParams *) display->parameters;
		if ( params->ogl_window.hWnd == window ) {
			OglActivate( display );
			OglDisplayRedraw( display );
		}
		display = display->next;
	}

}

local void _print_ogl ( void ) { 
  OglHardcopy( _ogl_display, "OglDisplay.ai" );
}

void	OglInit ( Display display ) {
  
  register OglParams	*params = (OglParams *) display->parameters;

  int width, height, x, y;

  if ( display->desired_width < 0 ) width = 900;
  else width = display->desired_width;
  if ( display->desired_height < 0 ) height = ( width * 3 ) / 4;
  else height = display->desired_height;

  x = display->desired_left;
  y = display->desired_top;
  
  StartOglWindow( display->name, x, y, width, height, NO, _refresh_ogl, _print_ogl );
  PrepOglWindow();

  // Store a pointer to the window and the device and rendering contexts.
  // These will be needed to active output to this display.
  GetOglWindow( &params->ogl_window );

  // Set the screen edges.
  display->left = 0.0;
  display->right = (float) width;
  display->top = (float) height;
  display->bottom = 0.0;

  // Set some parameters for drawing into a Adobe Illustrator compatible file.
  // This is pretty old stuff and may not work anymore.
  params->ai_scale = 0.5;
  params->ai_offset_x = ( 600 - params->ai_scale * width ) / 2.0;
  params->ai_offset_y = ( 850 - params->ai_scale * height ) / 2.0;
  params->cpy = 0;
  
  // Initialize the redraw linked list.
  DisplayInitCache( display );

  // Set the object of the refresh and print callbacks.
  _ogl_display = display;
  
}

/***************************************************************************/

void OglSwap ( Display display ) {

  Swap();

}

/***************************************************************************/

void OglActivate( Display display ) {

	register OglParams	*params = (OglParams *) display->parameters;
	SetOglWindow( &params->ogl_window );
	ActivateOglWindow();

}

/***************************************************************************/

void OglClose ( Display display ) {
 
	register OglParams	*params = (OglParams *) display->parameters;

	DisplayFreeCache( display );
	ShutdownOglWindowFromHandle( &params->ogl_window );
  
}

/***************************************************************************/

int	OglInput( Display display, float *x, float *y ) {
  
  register OglParams	*params = (OglParams *) display->parameters;
  int result = 0;
  
  return( result );
}

/***************************************************************************/

void	OglErase ( Display display) {
  
  register OglParams	*params = (OglParams *) display->parameters;
  
  glClear( GL_COLOR_BUFFER_BIT );
  if ( display->cache_active ) {
    DisplayInitCache( display );
  }
  
}

/***************************************************************************/

/* 
 * All operations are carried out in device  or pixel coordinates.
 */

#define PEN_RADIUS	1

void	OglPoint ( Display display, float x, float y) {
  
  register OglParams	*params = (OglParams *)display->parameters;
  
  glBegin( GL_POINTS );
  glVertex2f( x, y );
  glEnd();
  
  params->last_x = x;
  params->last_y = y;
  
  if ( display->cache_active ) {
    DisplayCacheItem *item;
    item = DisplayInsertCacheItem( display );
	item->token = point_token;
    item->param.point.x = x;
    item->param.point.y = y;
  }

  if ( params->cpy ) {
    fprintf( params->cpy, "%.2f %.2f m\n%.2f %.2f L\nS\n",
      ToAiX( x ), ToAiY( y ), ToAiX( x ) + 1, ToAiY( y ) + 1 );
  }
  
}

void	OglLine	( Display display, float x1, float y1, float x2, float y2) {
  
  register OglParams	*params = (OglParams *)display->parameters;
  
  glBegin( GL_LINES );
  glVertex2f( x1, y1 );
  glVertex2f( x2, y2 );
  glEnd();
  
  params->last_x = x2;
  params->last_y = y2;
  
  if ( display->cache_active ) {
    DisplayCacheItem *item;
    item = DisplayInsertCacheItem( display );
	item->token = line_token;
    item->param.line.x1 = x1;
    item->param.line.y1 = y1;
    item->param.line.x2 = x2;
    item->param.line.y2 = y2;
  }
  if ( params->cpy ) {
    fprintf( params->cpy, "%.2f %.2f m\n%.2f %.2f L\nS\n",
      ToAiX( x1 ), ToAiY( y1 ), ToAiX( x2 ), ToAiY( y2 ) );
  }
  
}

void	OglMoveTo ( Display display, float x, float y) {
  
  register OglParams	*params = (OglParams *)display->parameters;
  
  params->last_x = x;
  params->last_y = y;
  
  if ( display->cache_active ) {
    DisplayCacheItem *item;
    item = DisplayInsertCacheItem( display );
	item->token = moveto_token;
    item->param.point.x = x;
    item->param.point.y = y;
  }
  
  if ( params->cpy ) {
    fprintf( params->cpy, "%.2f %.2f m\n", ToAiX( x ), ToAiY( y ) );
  }
  
}

void	OglLineTo	( Display display, float x2, float y2 ) {
  
  register OglParams	*params = (OglParams *)display->parameters;
  
  float x1 = params->last_x;
  float y1 = params->last_y;
  
  glBegin( GL_LINES );
  glVertex2f( x1, y1 );
  glVertex2f( x2, y2 );
  glEnd();

  if ( display->cache_active ) {
    DisplayCacheItem *item;
    item = DisplayInsertCacheItem( display );
	item->token = lineto_token;
    item->param.point.x = x2;
    item->param.point.y = y2;
  }
  
  if ( params->cpy ) {
    fprintf( params->cpy, "%.2f %.2f m\n%.2f %.2f L\nS\n",
      ToAiX( x1 ), ToAiY( y1 ), ToAiX( x2 ), ToAiY( y2 ) );
  }

  params->last_x = x2;
  params->last_y = y2;

}

/***************************************************************************/

void	OglStartTrace	( Display display, float x, float y ) {

  register OglParams	*params = (OglParams *)display->parameters;

  glBegin( GL_LINE_STRIP );
  glVertex2f( x, y );

  if ( params->cpy ) {
    fprintf( params->cpy, "%.2f %.2f m\n", ToAiX( x ), ToAiY( y ) );
  }

}

void	OglContinueTrace	( Display display, float x, float y ) {
    
  register OglParams	*params = (OglParams *)display->parameters;

  glVertex2f( x, y );
  if ( params->cpy ) {
    fprintf( params->cpy, "%.2f %.2f L\n", ToAiX( x ), ToAiY( y ) );
  }
}

void	OglEndTrace	( Display display, float x, float y ) {

  register OglParams	*params = (OglParams *)display->parameters;

  glVertex2f( x, y );
  glEnd();
  
  if ( params->cpy ) {
    fprintf( params->cpy, "%.2f %.2f L\nS\n", ToAiX( x ), ToAiY( y ) );
  }

  params->last_x = x;
  params->last_y = y;
  
}

/***************************************************************************/

void	OglRectangle	( Display display, 
                     float x1,float y1, float x2, float y2) {
  
  register OglParams	*params = (OglParams *) display->parameters;
  
  
  
  glBegin( GL_LINE_LOOP );
  glVertex2f( x1, y1 );
  glVertex2f( x1, y2 );
  glVertex2f( x2, y2 );
  glVertex2f( x2, y1 );
  glEnd();
  
  params->last_x = x2;
  params->last_y = y2;
  
  if ( display->cache_active ) {
    
    DisplayCacheItem *item;
    
    item = DisplayInsertCacheItem( display );
	item->token = rectangle_token;
    item->param.rectangle.left = x1;
    item->param.rectangle.bottom = y1;
    item->param.rectangle.right = x2;
    item->param.rectangle.top = y2;
    
  }
  if ( params->cpy ) {
    fprintf( params->cpy, "%.2f %.2f m\n", ToAiX( x1 ), ToAiY( y1 ) );
    fprintf( params->cpy, "%.2f %.2f L\n", ToAiX( x1 ), ToAiY( y2 ) );
    fprintf( params->cpy, "%.2f %.2f L\n", ToAiX( x2 ), ToAiY( y2 ) );
    fprintf( params->cpy, "%.2f %.2f L\n", ToAiX( x2 ), ToAiY( y1 ) );
    fprintf( params->cpy, "%.2f %.2f L\n", ToAiX( x1 ), ToAiY( y1 ) );
    fprintf( params->cpy, "s\n" );
  }

}

void	OglFilledRectangle ( Display display, 
                          float x1, float y1, float x2, float y2) {
  
  register OglParams	*params = (OglParams *) display->parameters;
  
  
  if ( ( x1 < x2 && y1 < y2 ) || ( x1 > x2 && y1 > y2 ) ) {
    
    glBegin( GL_POLYGON );
		  glVertex2f( x1, y1 );
      glVertex2f( x2, y1 );
      glVertex2f( x2, y2 );
      glVertex2f( x1, y2 );
      //		  glVertex2f( x1, y1 );
      glEnd();
      
  }
  
  else {
    
    glBegin( GL_POLYGON );
		  glVertex2f( x1, y1 );
      glVertex2f( x1, y2 );
      glVertex2f( x2, y2 );
      glVertex2f( x2, y1 );
      //		  glVertex2f( x1, y1 );
      glEnd();
      
  }
  
  params->last_x = x2;
  params->last_y = y2;
  
  if ( display->cache_active ) {
    
    DisplayCacheItem *item;
    
    item = DisplayInsertCacheItem( display );
	item->token = filled_rectangle_token;
    item->param.rectangle.left = x1;
    item->param.rectangle.bottom = y1;
    item->param.rectangle.right = x2;
    item->param.rectangle.top = y2;
    
  }

  if ( params->cpy ) {
    fprintf( params->cpy, "%.2f %.2f m\n", ToAiX( x1 ), ToAiY( y1 ) );
    fprintf( params->cpy, "%.2f %.2f L\n", ToAiX( x1 ), ToAiY( y2 ) );
    fprintf( params->cpy, "%.2f %.2f L\n", ToAiX( x2 ), ToAiY( y2 ) );
    fprintf( params->cpy, "%.2f %.2f L\n", ToAiX( x2 ), ToAiY( y1 ) );
    fprintf( params->cpy, "F\n" );
  }

}

void	OglEraseRectangle ( Display display, 
                          float x1, float y1, float x2, float y2) {
  
  register OglParams	*params = (OglParams *) display->parameters;
  
  
  /* Erase screen to white. */
  glColor3f( 1.0, 1.0, 1.0 );

  if ( ( x1 < x2 && y1 < y2 ) || ( x1 > x2 && y1 > y2 ) ) {
    
    glBegin( GL_POLYGON );
		  glVertex2f( x1, y1 );
      glVertex2f( x2, y1 );
      glVertex2f( x2, y2 );
      glVertex2f( x1, y2 );
      //		  glVertex2f( x1, y1 );
      glEnd();
      
  }
  
  else {
    
    glBegin( GL_POLYGON );
		  glVertex2f( x1, y1 );
      glVertex2f( x1, y2 );
      glVertex2f( x2, y2 );
      glVertex2f( x2, y1 );
      //		  glVertex2f( x1, y1 );
      glEnd();
      
  }
  
  params->last_x = x2;
  params->last_y = y2;
  
  if ( display->cache_active ) {
    
    DisplayCacheItem *item;
    
    item = DisplayInsertCacheItem( display );
	item->token = erase_rectangle_token;
    item->param.rectangle.left = x1;
    item->param.rectangle.bottom = y1;
    item->param.rectangle.right = x2;
    item->param.rectangle.top = y2;
    
  }

  if ( params->cpy ) {
    /* Erase page to white. */
    fprintf( params->cpy, "0.0 0.0 0.0 0.0 K\n" );
    fprintf( params->cpy, "0.0 0.0 0.0 0.0 k\n" );
    fprintf( params->cpy, "%.2f %.2f m\n", ToAiX( x1 ), ToAiY( y1 ) );
    fprintf( params->cpy, "%.2f %.2f L\n", ToAiX( x1 ), ToAiY( y2 ) );
    fprintf( params->cpy, "%.2f %.2f L\n", ToAiX( x2 ), ToAiY( y2 ) );
    fprintf( params->cpy, "%.2f %.2f L\n", ToAiX( x2 ), ToAiY( y1 ) );
    fprintf( params->cpy, "F\n" );
  }

}

/***************************************************************************/

void	OglCircle	(Display display, float x, float y, float radius ) {
  
  register  OglParams	*params = (OglParams *) display->parameters;
  float     rx, ry, angle, angle_step = 2.0 * Pi / OGL_MAX_POLY_POINTS;
  
  glBegin( GL_LINE_LOOP );
  for ( angle = 0.0; angle < 2.0 * Pi; angle += angle_step ) {
    rx = x + radius * cos( angle );
    ry = y + radius * sin( angle );
    glVertex2f( rx, ry );
  }
  glEnd();

  if ( display->cache_active ) {
    DisplayCacheItem *item;
    item = DisplayInsertCacheItem( display );
	item->token = circle_token;
    item->param.circle.x = x;
    item->param.circle.y = y;
    item->param.circle.radius = radius;
  }

  if ( params->cpy ) {

    float left = ToAiX( x - radius );
    float right = ToAiX( x + radius );
    float top = ToAiY( y + radius );
    float bottom = ToAiY( y - radius );
    float aix = ToAiX( x );
    float aiy = ToAiY( y );

    fprintf( params->cpy, "%.2f %.2f m\n", right, aiy );
    fprintf( params->cpy, "%.2f %.2f %.2f %.2f %.2f %.2f c\n", 
      right, ( top + aiy ) / 2.0, ( aix + right ) / 2.0, top, aix, top );
    fprintf( params->cpy, "%.2f %.2f %.2f %.2f %.2f %.2f c\n", 
      ( aix + left ) / 2.0, top, left, ( top + aiy ) / 2.0, left, aiy );
    fprintf( params->cpy, "%.2f %.2f %.2f %.2f %.2f %.2f c\n", 
      left, ( aiy + bottom ) / 2.0, ( left + aix ) / 2, bottom, aix, bottom );
    fprintf( params->cpy, "%.2f %.2f %.2f %.2f %.2f %.2f c\n", 
      ( aix + right ) / 2.0, bottom, right, ( aiy + bottom ) / 2.0, right, aiy );
    fprintf( params->cpy, "s\n" );
  
  }

  
}

void	OglFilledCircle	(Display display, float x, float y, float radius )

{
  
  register  OglParams	*params = (OglParams *)display->parameters;
  float     rx, ry, angle, angle_step = Pi / 10.0;
  
  glBegin( GL_POLYGON );
  for ( angle = 0.0; angle < 2.0 * Pi; angle += angle_step ) {
    rx = x + radius * cos( angle );
    ry = y + radius * sin( angle );
    glVertex2f( rx, ry );
  }
  //  glVertex2f( x + radius, y );
  
  glEnd();
  
  if ( display->cache_active ) {
    
    DisplayCacheItem *item;
    
    item = DisplayInsertCacheItem( display );
	item->token = filled_circle_token;
    item->param.circle.x = x;
    item->param.circle.y = y;
    item->param.circle.radius = radius;
    
  }

  if ( params->cpy ) {

    float left = ToAiX( x - radius );
    float right = ToAiX( x + radius );
    float top = ToAiY( y + radius );
    float bottom = ToAiY( y - radius );
    float aix = ToAiX( x );
    float aiy = ToAiY( y );

    fprintf( params->cpy, "%.2f %.2f m\n", right, aiy );
    fprintf( params->cpy, "%.2f %.2f %.2f %.2f %.2f %.2f c\n", 
      right, ( top + aiy ) / 2.0, ( aix + right ) / 2.0, top, aix, top );
    fprintf( params->cpy, "%.2f %.2f %.2f %.2f %.2f %.2f c\n", 
      ( aix + left ) / 2.0, top, left, ( top + aiy ) / 2.0, left, aiy );
    fprintf( params->cpy, "%.2f %.2f %.2f %.2f %.2f %.2f c\n", 
      left, ( aiy + bottom ) / 2.0, ( left + aix ) / 2, bottom, aix, bottom );
    fprintf( params->cpy, "%.2f %.2f %.2f %.2f %.2f %.2f c\n", 
      ( aix + right ) / 2.0, bottom, right, ( aiy + bottom ) / 2.0, right, aiy );
    fprintf( params->cpy, "b\n" );
  
  }

}

/***************************************************************************/

void	OglStartPolygon ( Display display ) {
  
  register OglParams	*params = (OglParams *) display->parameters;
  params->vertex_count = 0;

  if ( display->cache_active ) {
    DisplayCacheItem *item;
    item = DisplayInsertCacheItem( display );
	item->token = start_polygon_token;
  }
  
}
void	OglAddVertex ( Display display, float x, float y ) {
  
  register OglParams	*params = (OglParams *) display->parameters;
  
  if ( params->vertex_count < OGL_MAX_POLY_POINTS ) {
    params->vertex[ params->vertex_count ].x = x;
    params->vertex[ params->vertex_count ].y = y;
    params->vertex_count++;
  }
  if ( display->cache_active ) {
    DisplayCacheItem *item;
    item = DisplayInsertCacheItem( display );
	item->token = add_vertex_token;
    item->param.point.x = x;
    item->param.point.y = y;
  }
}

void	OglOutlinePolygon ( Display display ) {
  
  register OglParams	*params = (OglParams *) display->parameters;
  
  int i;
  
  glBegin( GL_LINE_LOOP );
  for ( i = 0; i < params->vertex_count; i++ ) {
    glVertex2f( params->vertex[i].x, params->vertex[i].y );
  }  
  glEnd();

  if ( display->cache_active ) {
    DisplayCacheItem *item;
    item = DisplayInsertCacheItem( display );
	item->token = outline_polygon_token;
  }

  if ( params->cpy ) {
    fprintf( params->cpy, "%.2f %.2f m\n", 
      ToAiX( params->vertex[0].x ), ToAiY( params->vertex[0].y ) );
    for ( i = 1; i < params->vertex_count; i++ ) {
      fprintf( params->cpy, "%.2f %.2f l\n", 
        ToAiX( params->vertex[i].x ), ToAiY( params->vertex[i].y ) );
    }  
    fprintf( params->cpy, "s\n" );
  }

  
}


void	OglFillPolygon ( Display display ) {
  
  register OglParams	*params = (OglParams *) display->parameters;
  int i;
  
  glBegin( GL_POLYGON );
  
  for ( i = 0; i < params->vertex_count; i++ ) {
    glVertex2f( params->vertex[i].x, params->vertex[i].y );
  }
  glVertex2f( params->vertex[0].x, params->vertex[0].y );
  if ( display->cache_active ) {
    DisplayCacheItem *item;
    item = DisplayInsertCacheItem( display );
	item->token = fill_polygon_token;
  }
  
  glEnd();
  
  if ( params->cpy ) {
    fprintf( params->cpy, "%.2f %.2f m\n", 
      ToAiX( params->vertex[0].x ), ToAiY( params->vertex[0].y ) );
    for ( i = 1; i < params->vertex_count; i++ ) {
      fprintf( params->cpy, "%.2f %.2f l\n", 
        ToAiX( params->vertex[i].x ), ToAiY( params->vertex[i].y ) );
    }  
    fprintf( params->cpy, "f\n" );
  }
  
}

/***************************************************************************/

void	OglText	( Display display, char *string, float x, float y, double dir ) {
  
  register OglParams	*params = (OglParams *) display->parameters;
  double c, s;
  
  glprintf( (int) x, (int) y, ogl_font_height, "%s\n", string );
  if ( display->cache_active ) {
    
    DisplayCacheItem *item;
    item = DisplayInsertCacheItem( display );
	item->token = text_token;
    item->param.text.x = x;
    item->param.text.y = y;
    item->param.text.dir = dir;
    item->param.text.string = strdup( string );
    
  }
  

  if ( params->cpy ) {
    c = cos( dir );
    s = sin( dir );
    fprintf( params->cpy, "[%.2f %.2f %.2f %.2f %.2f %.2f] e\n", 
      c, s, - s, c, ToAiX( x ), ToAiY( y ) );
    fprintf( params->cpy, "(%s ) t\nT\n", string );
  }
}

float	OglTextWidth ( Display display, char *string ) {
 
  unsigned int i;
  float add = 0.0, adjust = 0.25;
  for ( i = 0; i < strlen( string ); i++ ) {
    if ( string[i] >= 'A' && string[i] <= 'Z' ) add += adjust;
  }
  return( (float) ( ogl_font_width * strlen( string ) ) + add );
  
}

float	OglTextHeight ( Display display, char *string ) {
  
  return ( (float) ogl_font_height );
  
}

/***************************************************************************/

void	OglAlu	( Display display, int alu) {
  
  register OglParams	*params = (OglParams *) display->parameters;
  
}

void	OglLineStyle ( Display display, int style ) {
  
  register OglParams	*params = (OglParams *) display->parameters;
  
}

void	OglLinePattern	( Display display, int pattern ) {
  
  register OglParams	*params = (OglParams *) display->parameters;
  
}

void	OglPenSize ( Display display, float size) {
  
  register OglParams	*params = (OglParams *) display->parameters;
  if ( display->cache_active ) {
    
    DisplayCacheItem *item;
    
    item = DisplayInsertCacheItem( display );
	item->token = pen_token;
    item->param.pen = size;
  }
  if ( params->cpy ) {
    fprintf( params->cpy, "%.3f w\n", size );
  }
  
}

/***************************************************************************/

// Color 

#define RGB_ERASE 0xff000000
#define RGB_BLACK 0xff010000
#define SUBSTITUTE_FOR_BLACK 0xff0000cc

typedef	struct {
  float red;
  float green;
  float blue;
} OglRGBColor;

OglRGBColor	OglColorTable[] = {
  
  { 0, 0, 0 },	/* Black	*/
  { 1, 0, 0 },	/* Red		*/
  { 0, 1, 0 },	/* Green	*/
  { 1, 1, 0 },	/* Yellow	*/
  { 0, 0, 1 },	/* Blue		*/
  { 1, 0, 1 },	/* Magenta	*/
  { 0, 1, 1 },	/* Cyan		*/
  { 1, 1, 1 },	/* White	*/
  
  { .125, .125, .125 },	/* Grey1	*/
  { .25, .25, .25 },	/* Grey2	*/
  { .375, .375, .375 },	/* Grey3	*/
  { .5,.5, .5 },		/* Grey4	*/
  { .625, .625, .625 },	/* Grey5	*/ 
  { .75, .75, .75 },	/* Grey6	*/
  { .875, .875, .875 },	/* Grey7	*/
  { 1.0, 1.0, 1.0 },	/* Grey8	*/
  
  { 0, 0, 0 },   /* TRANSPARENT */
  
};

void	OglColor ( Display display, int color) {
  
  register OglParams	*params = (OglParams *)display->parameters;
  
  glColor3f( OglColorTable[color].red, 
    OglColorTable[color].green,
    OglColorTable[color].blue );

  if ( display->cache_active ) {
    
    DisplayCacheItem *item;
    
    item = DisplayInsertCacheItem( display );
	item->token = color_token;
    item->param.color = color;
    
  }
  if ( params->cpy ) {
    fprintf( params->cpy, "%.3f %.3f %.3f 0 k\n", 
      1.0 - OglColorTable[color].red,
      1.0 - OglColorTable[color].green,
      1.0 - OglColorTable[color].blue );
    fprintf( params->cpy, "%.3f %.3f %.3f 0 K\n", 
      1.0 - OglColorTable[color].red,
      1.0 - OglColorTable[color].green,
      1.0 - OglColorTable[color].blue );
  }

}

void	OglColorRGB ( Display display, float r, float g, float b ) {
  
  register OglParams	*params = (OglParams *)display->parameters;
  
  glColor3f( r, g, b );

  if ( display->cache_active ) {
    
    DisplayCacheItem *item;
    
    item = DisplayInsertCacheItem( display );
	item->token = rgb_token;
    item->param.rgb.r = r;
    item->param.rgb.g = g;
    item->param.rgb.b = b;
    
  }
  if ( params->cpy ) {
    fprintf( params->cpy, "%.3f %.3f %.3f 0 k\n", r, g, b );
    fprintf( params->cpy, "%.3f %.3f %.3f 0 K\n", r, g, b );
  }

}


