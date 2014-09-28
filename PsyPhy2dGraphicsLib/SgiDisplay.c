#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#include <fmclient.h>

#include <gl/gl.h>
#include <gl/device.h>
#include <psgl/gl.h> 

#include <useful.h>
#include "Graphics.h"
#include "SgiDisplay.h"

/***************************************************************************/

SgiParams	_sgi_params = {"SGI 2D Display", 0, 0};
struct _display	_SgiDisplay = {
  "Sgi Display",
  20, 20, 1040, 735,
  SgiPoint, SgiLine, SgiMoveTo, SgiLineTo,
  SgiText, SgiTextWidth, SgiTextHeight,
  SgiRectangle, SgiFilledRectangle,
  SgiCircle, SgiFilledCircle,

  SgiStartPolygon, 
  SgiAddVertex, 
  SgiOutlinePolygon, 
  SgiFillPolygon,

  SgiErase, SgiEraseRectangle,
  SgiLineStyle, SgiLinePattern,
  SgiColor, SgiAlu, 
  SgiPenSize,
  SgiInit, SgiSwap, SgiClose, SgiHardcopy, SgiInput,
  SOLID,						/* Line Pattern */
  SET,							/* ALU */
  FOREGROUND,						/* Color */
  FALSE,						/* Black and White */
  2,							/* Symbol Size (radius) */
  -1, -1,						/* Desired Width and Height */
  NULL, NULL, NO,					/* Redraw cache */
  &_sgi_params
};

Display		SgiDisplay = &_SgiDisplay;


#define RGB_ERASE 0xff000000
#define RGB_BLACK 0x00010000
#define SUBSTITUTE_FOR_BLACK 0xff0000cc

RGBColor	SgiColorTable[] = {

  0x00010000,	/* Black	*/
  0xff0000ff,	/* Red		*/
  0xff00ff00,	/* Green	*/
  0xff00ffff,	/* Yellow	*/
  0xffff2222,	/* Blue		*/
  0xffff00ff,	/* Magenta	*/
  0xffffff00,	/* Cyan		*/
  0xffffffff,	/* White	*/

  0xff111111,	/* Grey1	*/
  0xff333333,   /* Grey2	*/
  0xff555555,	/* Grey3	*/
  0xff777777,	/* Grey4	*/
  0xff999999,	/* Grey5	*/
  0xffbbbbbb,	/* Grey6	*/
  0xffdddddd,	/* Grey7	*/
  0xffffffff,	/* Grey8	*/

  0x00000000,   /* TRANSPARENT */

};

/* Line Patterns */
#define SOLID            0
#define DASH             1
#define DOT              2
#define DASH_DOT         3
#define DASH_DOT_DOT     4
#define LONG_DASH        5
#define CENTER_DASH      6
#define CENTER_DASH_DASH 7
#define PATTERNS 8

unsigned short SgiPatternTable[] = {
  0xffff,
  0x00ff,
  0xcccc,
  0xff30,
  0xff36,
  0xfffc,
  0x3ffc,
  0x3c3c };

/***************************************************************************/

typedef struct {

  long window;
  int  code;
  float  x1;
  float  y1;
  float  x2;
  float  y2;
  char *str;
  unsigned long color;
  
  void *next;

} SgiDisplayCacheItem;

enum { CODE_CLEAR, CODE_POINT, CODE_MOVE, CODE_DRAW, 
       CODE_ERASE_RECTANGLE, CODE_FILLED_RECTANGLE, CODE_RECTANGLE,
       CODE_CIRCLE, CODE_FILLED_CIRCLE,
       CODE_COLOR, CODE_PATTERN, CODE_LINEWIDTH, CODE_STRING,
       CODE_BGNTRACEPOLY, CODE_BGNFILLPOLY,
       CODE_VERTEX, 
       CODE_ENDTRACEPOLY, CODE_ENDFILLPOLY };

local SgiDisplayCacheItem root = { CODE_CLEAR, 0, 0, 0, 0, 0 };
local SgiDisplayCacheItem *current;
local cache_enable = TRUE;

local fmfonthandle _sgi2d_font1, _sgi2d_fontx;
local int _sgi2d_font_height = 12;
local char *default_font = "Times-Roman";
local double screen_mag = 1.0;

local init_cache( long window ) {
  current = &root;
  root.window = window;
  root.next = 0;
  root.color = BLACK;
}

void SgiCacheEnable ( void ) {
  cache_enable = YES;
}

void SgiCacheDisable ( void ) {
  cache_enable = NO;
}

local void add_to_cache( long window, int code, 
						 float x1, float y1, float x2, float y2,
						 char *str, unsigned long color ) {

  SgiDisplayCacheItem *new;

  if ( !cache_enable ) return;

  if ( code == CODE_CLEAR ) {
    init_cache( window );
  }
  else {
    new = malloc( sizeof( SgiDisplayCacheItem ) );
    new->code = code;
    new->x1 = x1;
    new->y1 = y1;
    new->x2 = x2;
    new->y2 = y2;
    if ( str ) new->str = strdup( str );
    else str = 0;
    new->color = color;
    new->next = 0;
    new->window = window;
    current->next = new;
    current = new;
  }

}

typedef	union {
  struct { 
    unsigned char a;
    unsigned char b;
    unsigned char g;
    unsigned char r;
  } rgba;
  unsigned long word;
} SgiRGBColor;

SgiDisplayRedraw ( void ) {

  extern int psgl_enabled;

  long width, height;
  long counter = 1;
  float  current_subdivide;

  float vect[2];

  SgiDisplayCacheItem *ptr;

  ptr = &root;

  fprintf( stderr, "Redraw " );

  winset( ptr->window );
  getsize(&width, &height);
  ortho2( 0, width, 0, height );
  viewport( 0, width, 0, height );

  /* 
   * Everything should be 2D, so there is no
   * need to break things up into little pieces.
   */
  current_subdivide = psgl_getsubdivide();
  psgl_setsubdivide( 10000.0 );

  while ( ptr ) {

    if ( !(counter % 1000) ) fprintf( stderr, "." );
    counter++;

    switch ( ptr->code ) {
 
    case CODE_CLEAR:
      cpack( ptr->color );
      clear();
      break;

    case CODE_MOVE:
      move2( ptr->x1, ptr->y1 );
      break;

    case CODE_POINT:
      pnt2( ptr->x1, ptr->y1 );
	  circf( ptr->x1, ptr->y1, 1.0 );
      break;

    case CODE_DRAW:
      draw2( ptr->x1, ptr->y1 );
      break;

    case CODE_PATTERN:
      setlinestyle( ptr->color );
      break;

    case CODE_LINEWIDTH:
      linewidthf( ptr->x1 );
      break;

    case CODE_COLOR:
      if ( !psgl_enabled ) {


		if ( ptr->color == RGB_BLACK ) cpack( SUBSTITUTE_FOR_BLACK );
		else cpack( ptr->color );
      }
      else {

		SgiRGBColor *pp, packed;
		pp = (SgiRGBColor *) &(ptr->color);

		if ( pp->rgba.r == pp->rgba.g && pp->rgba.g == pp->rgba.b ) {
		  packed.rgba.r = packed.rgba.g = packed.rgba.b = 
			256 - (int) pp->rgba.g;
		  packed.rgba.a = pp->rgba.a;
		  cpack( packed.word );
		}
		else cpack( ptr->color );
      }
      break;

    case CODE_STRING:
      cmov2( ptr->x1, ptr->y1 );
      if ( _sgi2d_fontx ) {
	fmsetfont( _sgi2d_fontx );
	fmprstr( ptr->str );
      }
      else charstr( ptr->str );
      break;

    case CODE_FILLED_RECTANGLE:
      rectf( ptr->x1, ptr->y1, ptr->x2, ptr->y2 );
      break;

    case CODE_RECTANGLE:
      recti( ptr->x1, ptr->y1, ptr->x2, ptr->y2 );
      break;

    case CODE_FILLED_CIRCLE:
      circf( ptr->x1, ptr->y1, ptr->x2 );
      break;

    case CODE_CIRCLE:
      circ( ptr->x1, ptr->y1, ptr->x2 );
      break;

    case CODE_ERASE_RECTANGLE:
      cpack( RGB_ERASE );
      rectf( ptr->x1, ptr->y1, ptr->x2, ptr->y2 );
      break;
	  
    case CODE_BGNTRACEPOLY:
      bgnclosedline();
      break;

    case CODE_BGNFILLPOLY:
      bgnpolygon();
      break;

    case CODE_VERTEX:
      vect[0] = ptr->x1;
      vect[1] = ptr->y1;
      v2f( vect );
      break;

    case CODE_ENDTRACEPOLY:
      endclosedline();
      break;
	  
    case CODE_ENDFILLPOLY:
      endpolygon();
      break;
	  
    default:
      fprintf( stderr, "Unknown function code %x for cache.\n", ptr->code );
      break;

    }

    ptr = ptr->next;

  }

  psgl_setsubdivide( current_subdivide );
  fprintf( stderr, " OK!\n" );
}

      
/***************************************************************************/

local void define_events () {

  unqdevice( INPUTCHANGE );
   
  /* Local keys processed by parse_key(). */

  qdevice( PKEY );
  qdevice( SPACEKEY );
  qdevice( REDRAW );
  qdevice( WINTHAW );
  qdevice( ESCKEY );

  qreset();

}

/***************************************************************************/

local char psroot[256] = "Graphics";
void SgiRootPS ( char *root ) {
  strcpy( psroot, root );
}

/***************************************************************************/

void SgiHardcopy ( Display display ) {

  char psfile[256];
  int  psfilecnt = 0;

  for ( psfilecnt = 0; psfilecnt < 100; psfilecnt++ ) {
    sprintf( psfile, "%s.%02d.ps", psroot, psfilecnt );
    if ( access( psfile, F_OK ) ) break;
  }
  fprintf( stderr, "Postscript file: %s ... open.\n", psfile );
  fflush( stderr );
  psgl_open( psfile );
  SgiDisplayRedraw();
  swapbuffers();
  fprintf( stderr, "Postscript file: %s ... closed.\n", psfile );


}

/***************************************************************************/

void SgiHardcopyPict ( Display display ) {

  char psfile[256];
  int  psfilecnt = 0;

  for ( psfilecnt = 0; psfilecnt < 100; psfilecnt++ ) {
    sprintf( psfile, "%s%02d.pct", psroot, psfilecnt );
    if ( access( psfile, F_OK ) ) break;
  }
  fprintf( stderr, "Pict file: %s ... open.\n", psfile );
  fflush( stderr );
  psgl_open_pict( psfile );
  SgiDisplayRedraw();
  swapbuffers();
  fprintf( stderr, "Pict file: %s ... closed.\n", psfile );


}
/***************************************************************************/

void *SgiSelectFont( char *font_name, int point_size ) {

  _sgi2d_font1 = fmfindfont( font_name );
  if ( !_sgi2d_font1 ) {
	fprintf( stderr, "Error finding font %s.\n", font_name );
  }
  else {
    fmfontinfo info;
    _sgi2d_fontx = fmscalefont( _sgi2d_font1, (double) point_size * screen_mag );
    fmgetfontinfo( _sgi2d_fontx, &info );
    /* Knock off 2 pixels to get a tighter line spacing. */
    _sgi2d_font_height = info.height - 2;
    fmsetfont( _sgi2d_fontx );
  }

  return( _sgi2d_font1 );

}

/***************************************************************************/

void	SgiInit ( Display display ) {

  register SgiParams	*params = display->parameters;

  long    window_width, window_height;
  long    screen_width, screen_height;
  long    screen_width_mm, screen_height_mm;

  long	  width, height;
  char    window_name[256];

  double  pixels_per_mm;

  long preferred_top = 200;
  long preferred_bottom = 735;
  long preferred_left = 200;
  long preferred_right = 1024;

  char *page;

  int i;
    
  if ( getgdesc( GD_BITS_NORM_DBL_RED ) == 0 ) {
    fprintf(stderr, "Double buffered RBG not available.\n");
    exit(-1);
  }

  /* If a window already exists, use it. */
  params->window = winget();
  if ( params->window < 0 ) {

    screen_width = getgdesc( GD_XPMAX );
    screen_height = getgdesc( GD_YPMAX );

    /* Default is an A4 portrait page with 1cm margins. */

    screen_width_mm = getgdesc( GD_XMMAX );
    screen_height_mm = getgdesc( GD_YMMAX );
    pixels_per_mm = (double) screen_width / (double) screen_width_mm;

    height = 270 * pixels_per_mm;
    width= 190 * pixels_per_mm;

    if ( page = getenv( "VIEW_PAGE" ) ) {

      if ( !strcmp( page, "LANDSCAPE" ) ) {

		/* Substitute an A4 portrait page with 1cm margins. */

		width = 270 * pixels_per_mm;
		height= 190 * pixels_per_mm;


      }
      else if ( !strcmp( page, "FULL" ) ) {

		/* Substitute an A4 portrait page with 1cm margins. */

		width = screen_width - 40;
		height= screen_height - 40;


      }
      else if ( !strncmp( page, "CUSTOM", strlen( "CUSTOM" ) ) ) {

		float desired_width, desired_height;
		float pixel_factor_x, pixel_factor_y;
		char units[256];


		/* Specify the size of the desired image on the page. */
		sscanf( page, "CUSTOM%fx%f%s", &desired_width, &desired_height, units );
		if ( tolower( units[0] ) == 'i' ) {
		  psgl_setsizeinches( desired_width, desired_height, PSGL_STRETCH );
		  width = desired_width * screen_width / screen_width_mm * 25.4;
		  height = width * desired_height / desired_width;
		}
		else {
		  psgl_setsizecm( desired_width, desired_height, PSGL_STRETCH );
		  width = desired_width * screen_width / screen_width_mm * 10.0;
		  height = width * desired_height / desired_width;
		}
		fprintf( stderr, "Custom window size: %d %d\n", width, height );

      }

    }
    else if ( display->desired_width > 0.0 && display->desired_height > 0.0 ) {
      psgl_setsizeinches( display->desired_width, 
						  display->desired_height, 
						  PSGL_STRETCH );
      width = display->desired_width * screen_width / screen_width_mm * 25.4;
      height = width * display->desired_height / display->desired_width;
    }

    /*
     * Unless overidden by the environment variable, magnify the
     * screen image as much as possible. This will maintain
     * the same size image on paper, while increasing resolution
     * on the screen.
     */
    if ( page = getenv( "VIEW_MAGNIFY" ) ) {
      sscanf( page, "%lf", &screen_mag );
    }
    else {

      screen_mag = (double) screen_width / (double) width;
      if (( (double) screen_height / (double) height ) < screen_mag ) {
		screen_mag = (double) screen_height / (double) height;
      }
      screen_mag = floor( screen_mag );


    }
    width *= screen_mag;
    height *= screen_mag;
    display->symbol_radius *= screen_mag;
    if ( width > screen_width - 40 ) width -= 40;
    if ( height > screen_height - 40 ) height -= 40;

    fprintf( stderr, "%d %d %f %d %d\n",
			 screen_width, screen_height, screen_mag, width, height );
    preferred_right = screen_width - 20;
    preferred_left = preferred_right - width;
    preferred_top = screen_height - 20;
    preferred_bottom = preferred_top - height;

    prefposition( preferred_left, preferred_right, 
				  preferred_top, preferred_bottom );

    /* Indicate the magnification, if any, in the window name. */
    if ( screen_mag != 1.0 ) {
	  sprintf( window_name, "%s X %.0f", params->name, screen_mag );
	}
    else strcpy(window_name, params->name );

    /* 
     * Open a window for the display.
     * Keep the program in the foreground, i.e. don't 
     * start a new process for the window.
     */
    foreground(); 
    params->window = winopen( window_name );

  }

  /* Find out what the window manager really gave us. */
  getsize(&window_width, &window_height);
  display->left = 0;
  display->right = window_width - 1;
  display->bottom = 0;
  display->top = window_height - 1;

  drawmode( NORMALDRAW );
  RGBmode();
  zbuffer( TRUE );
  doublebuffer();
  gconfig();

  ortho2( -0.5, window_width + 0.5, -0.5, window_height + 0.5 );
  viewport( 0, window_width, 0, window_height );

  /* Antialiasing */
  subpixel( TRUE );

  /* 
     Install hooks for generation of postscript output.
     <ctrl><shift>-<Print Screen> will cause the next screen
     update to go to a file instead of to the screen.
     */
  psgl_install();


  /* 
     The psgl system divides surfaces into small triangles,
     each with a constant color.  This call sets the maximum
     size of each polygon side IN PIXELS.  Psgl automatically
     subdivides polygons that are greater than this size.

     Note the interaction with the size of the window defined
     above, due to the fact that this threshold is in pixels.
     The bigger the window, the smaller the postscript triangles.
     */
  psgl_setsubdivide( 5.0 );

  /*
    The psgl hardcopy system uses double buffering and swapbuffer 
    to decide when to output to the file instead of the screen.
    But I like to see how things are drawn as the occur, not
    the final effect when swapbuffers() is called.  Here I 
    enable the front buffer for drawing also.  That way the 
    visible buffer gets updated at the same time.

    This may slow things down, so it might be good to supress 
    it in the future.
    */
  frontbuffer( TRUE );

  define_events();
  init_cache( params->window );

  SgiErase( display );
  SgiColor( display, WHITE );
  SgiPenSize( display, 1.0 );
  fminit();
  SgiSelectFont( default_font, 12 );

  for ( i = 1; i < PATTERNS; i++ ) {
    deflinestyle( i, SgiPatternTable[i] );
    fprintf( stderr, "%d %04x\n", i, SgiPatternTable[i] );
  }

}

/***************************************************************************/

void SgiClose ( Display display ) {

  register SgiParams	*params = display->parameters;
  long  device;
  short data;
  static Shift = 0;

  char psfile[256];
  int  psfilecnt = 0;
  while ( 1 ) {

    /* Wait for something to happen. */
    while( !qtest() );
    device = qread( &data );

    if ( device == LEFTSHIFTKEY || device == RIGHTSHIFTKEY ) {
      Shift = data;
    }

    else if ( data ) {

      switch( (device = qread( &data )) ) {

      case PKEY:

		if ( Shift ) SgiHardcopyPict( display );
		else SgiHardcopy( display );
		break;

      case ESCKEY:
		gexit();
		exit( 0 );
		break;

      default:

		/* Redraw the display. */
		SgiDisplayRedraw();
		swapbuffers();

		break;

      }
    }
    else {
      SgiDisplayRedraw();
      swapbuffers();
    }
  }
}
/***************************************************************************/

void SgiSwap ( Display display ) {

  register SgiParams	*params = display->parameters;

  swapbuffers();

}

/***************************************************************************/

int	SgiInput( Display display, float *x, float *y ) {

  register SgiParams	*params = display->parameters;
  long  device;
  short data;
  static Shift = 0;
  int   quit = NO, result = NULL;

  char psfile[256];
  int  psfilecnt = 0;

  static long x_origin, y_origin, x_mouse, y_mouse;
  getorigin( &x_origin, &y_origin );

  qdevice( MOUSEX );
  qdevice( MOUSEY );
  qdevice( LEFTMOUSE );
  qdevice( RIGHTMOUSE );
  qdevice( MIDDLEMOUSE );

  while ( !quit ) {

    switch( (device = qread( &data )) ) {

    case PKEY:

      if ( Shift ) SgiHardcopyPict( display );
      else SgiHardcopy( display );
      break;

    case MOUSEX:
      x_mouse = data - x_origin;
      break;

    case MOUSEY:
      y_mouse = data - y_origin;
      break;

    case LEFTMOUSE:
      if ( data ) {
		*x = x_mouse;
		*y = y_mouse;
		result = DISPLAY_LEFT_MOUSE;
		quit = YES;
      }
      break;

    case MIDDLEMOUSE:
      if ( data ) {
		*x = x_mouse;
		*y = y_mouse;
		result = DISPLAY_MIDDLE_MOUSE;
		quit = YES;
      }
      break;

    case RIGHTMOUSE:
      if ( data ) {
		*x = x_mouse;
		*y = y_mouse;
		result = DISPLAY_RIGHT_MOUSE;
		quit = YES;
      }
      break;

    case ESCKEY:
      return( DISPLAY_ESCAPE );
      break;

    default:
      SgiDisplayRedraw();
      break;

    }
  }

  unqdevice( MOUSEX );
  unqdevice( MOUSEY );
  unqdevice( LEFTMOUSE );
  unqdevice( RIGHTMOUSE );
  unqdevice( MIDDLEMOUSE );

  return( result );
}
    



/***************************************************************************/

/* 
	All operations are carried out in device  or pixel coordinates.
*/

void	SgiPoint ( Display display, float x, float y) {

  register SgiParams	*params = (SgiParams *)display->parameters;

  winset( params->window );
  pnt2( x, y );
  circf( x, y, params->linewidth );
  add_to_cache( params->window, CODE_POINT, x, y, 0, 0, 0, 0 );
	
}

void	SgiLine	( Display display, float x1, float y1, float x2, float y2) {

  register SgiParams	*params = (SgiParams *)display->parameters;

  winset( params->window );
  move2( x1, y1 );
  draw2( x2, y2 );

  add_to_cache( params->window, CODE_MOVE, x1, y1, 0, 0, 0, 0 );
  add_to_cache( params->window, CODE_DRAW, x2, y2, 0, 0, 0, 0 );
	
}

void	SgiMoveTo ( Display display, float x1, float y1) {

  register SgiParams	*params = (SgiParams *)display->parameters;

  winset( params->window );
  move2( x1, y1 );
  add_to_cache( params->window, CODE_MOVE, x1, y1, 0, 0, 0, 0 );

	
}

void	SgiLineTo	( Display display, float x2, float y2 ) {

  register SgiParams	*params = (SgiParams *)display->parameters;

  winset( params->window );
  draw2( x2, y2 );
  add_to_cache( params->window, CODE_DRAW, x2, y2, 0, 0, 0, 0 );

	
}

void	SgiText	( Display display, char *string, 
				  float x, float y, double dir ) {
	
  register SgiParams	*params = display->parameters;
	
  winset( params->window );
  cmov2( x, y );
  if ( _sgi2d_fontx ) {
    fmsetfont( _sgi2d_fontx );
    fmprstr( string );
  }
  else charstr( string );
  add_to_cache( params->window, CODE_STRING, x, y, 0, 0, string, 0 );
 
}

float	SgiTextWidth ( Display display, char *string ) {
	
  if ( _sgi2d_fontx ) return( fmgetstrwidth( _sgi2d_fontx, string ) );
  else return ( strwidth( string ) );
	
}
	
float	SgiTextHeight ( Display display, char *string ) {
	
  return ( _sgi2d_font_height );
	
}
	
	
void	SgiRectangle	( Display display, 
						  float x1,float y1, float x2, float y2) {
	
  register SgiParams	*params = display->parameters;
  winset( params->window );
  recti( x1, y1, x2, y2 );
  add_to_cache( params->window, CODE_RECTANGLE, x1, y1, x2, y2, 0, 0 );


}

void	SgiFilledRectangle ( Display display, 
							 float x1, float y1, float x2, float y2) {
	
  register SgiParams	*params = display->parameters;
  winset( params->window );
  rectf( x1, y1, x2, y2 );
  add_to_cache( params->window, CODE_FILLED_RECTANGLE, x1, y1, x2, y2, 0, 0 );

}

void	SgiCircle	(Display display, float x, float y, float radius ) {

  register SgiParams	*params = (SgiParams *)display->parameters;
  winset( params->window );
  circi( x, y, radius );
  add_to_cache( params->window, CODE_CIRCLE, x, y, radius, 0, 0, 0 );
		
}

void	SgiFilledCircle	(Display display, float x, float y, float radius )

{
  register SgiParams	*params = (SgiParams *)display->parameters;
  winset( params->window );
  circf( x, y, radius );
  add_to_cache( params->window, CODE_FILLED_CIRCLE, x, y, radius, 0, 0, 0 );
}

void	SgiEraseRectangle ( Display display, 
							float x1, float y1, float x2, float y2) {

  register SgiParams	*params = (SgiParams *)display->parameters;

  winset( params->window );
  cpack( RGB_ERASE );
  rectf( x1, y1, x2, y2 );
  add_to_cache( params->window, CODE_ERASE_RECTANGLE, x1, y1, x2, y2, 0, 0 );

}

void	SgiErase ( Display display) {

  register SgiParams	*params = display->parameters;

  winset( params->window );
  cpack( RGB_ERASE );
  clear();
  zclear();
  add_to_cache( params->window, CODE_CLEAR, 0, 0, 0, 0, 0, 0 );

}

void	SgiAlu	( Display display, int alu) {

  register SgiParams	*params = display->parameters;
  winset( params->window );

}

void	SgiLineStyle ( Display display, int style ) {
	
  register SgiParams	*params = display->parameters;
  winset( params->window );

  if ( display->bw ) SgiLinePattern( display, StyleToPattern( style ) );
  else SgiColor( display, StyleToColor( style ) );
 
}

void	SgiLinePattern	( Display display, int pattern ) {
	
  register SgiParams	*params = display->parameters;
  winset( params->window );

  setlinestyle( pattern );
  add_to_cache( params->window, CODE_PATTERN, 0, 0, 0, 0, 0, pattern );

}

void	SgiColor ( Display display, int color) {
	
  extern int psgl_enabled;

  unsigned int packed;

  register SgiParams	*params = (SgiParams *)display->parameters;
  winset( params->window );

  if ( display->bw ) packed = SgiColorTable[ BLACK ];
  else packed =  SgiColorTable[color];

  /* Change black to a dark red on the screen, black on paper. */
  if ( !psgl_enabled ) {
    if ( packed == RGB_BLACK ) cpack( SUBSTITUTE_FOR_BLACK );
    else cpack( packed );
  }
  else cpack( packed );

  add_to_cache( params->window, CODE_COLOR, 0, 0, 0, 0,	0, packed );
 
}

void	SgiPenSize ( Display display, float size) {
	
  register SgiParams	*params = (SgiParams *)display->parameters;
  winset( params->window );

  add_to_cache( params->window, CODE_LINEWIDTH, size, 0, 0, 0,	0, 0 );
  params->linewidth = size;
  linewidthf( size );
  

}

void	SgiStartPolygon ( Display display ) {

  register SgiParams	*params = display->parameters;
  winset( params->window );

  params->vertex_count = 0;

}
void	SgiAddVertex ( Display display, float x, float y ) {

  register SgiParams	*params = display->parameters;
  winset( params->window );

  if ( params->vertex_count >= SGI_MAX_POLY_POINTS ) return;
  params->vertex[ params->vertex_count ].x = x;
  params->vertex[ params->vertex_count ].y = y;
  params->vertex_count++;


}

void	SgiOutlinePolygon ( Display display ) {

  float vect[2];
  int i;

  register SgiParams	*params = display->parameters;
  winset( params->window );

  bgnclosedline();
  add_to_cache( params->window, CODE_BGNTRACEPOLY, 0, 0, 0, 0, 0, 0 );
  for ( i = 0; i < params->vertex_count; i++ ) {
    vect[0] = params->vertex[i].x;
    vect[1] = params->vertex[i].y;
    v2f( vect );
    add_to_cache( params->window, CODE_VERTEX, 
				  vect[0], vect[1], 0, 0, 0, 0 );  
  }
  endclosedline();
  add_to_cache( params->window, CODE_ENDTRACEPOLY, 0, 0, 0, 0, 0, 0 );

}
void	SgiFillPolygon ( Display display ) {

  float vect[2];
  int i;

  register SgiParams	*params = display->parameters;
  winset( params->window );

  bgnpolygon();
  add_to_cache( params->window, CODE_BGNFILLPOLY, 0, 0, 0, 0, 0, 0 );
  for ( i = 0; i < params->vertex_count; i++ ) {
    vect[0] = params->vertex[i].x;
    vect[1] = params->vertex[i].y;
    v2f( vect );
    add_to_cache( params->window, CODE_VERTEX, 
				  vect[0], vect[1], 0, 0, 0, 0 );  
  }
  endpolygon();
  add_to_cache( params->window, CODE_ENDFILLPOLY, 0, 0, 0, 0, 0, 0 );

}


