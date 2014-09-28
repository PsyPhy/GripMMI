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
#include <preprocess.h>
#include "Graphics.h"
#include "SgiMapDisplay.h"

/***************************************************************************/

SgiMapParams	_sgi_map_params = {"SGI COLORMAP 2D Display", 0};
struct _display	_SgiMapDisplay = {
  "SgiMap Display",
  20, 20, 1040, 735,
  SgiMapPoint, SgiMapLine, SgiMapMoveTo, SgiMapLineTo,
  SgiMapText, SgiMapTextWidth, SgiMapTextHeight,
  SgiMapRectangle, SgiMapFilledRectangle,
  SgiMapCircle, SgiMapFilledCircle,

  SgiMapStartPolygon, 
  SgiMapAddVertex, 
  SgiMapOutlinePolygon, 
  SgiMapFillPolygon,

  SgiMapErase, SgiMapEraseRectangle,
  SgiMapLineStyle, SgiMapLinePattern,
  SgiMapColor, SgiMapAlu, 
  SgiMapPenSize,
  SgiMapInit, SgiMapClose, SgiMapHardcopy, SgiMapInput,
  SOLID,						/* Line Pattern */
  SET,							/* ALU */
  FOREGROUND,					/* Color */
  FALSE,						/* Black and White */
  3,							/* Symbol Size (radius) */
  -1, -1,						/* Desired Width and Height */
  NULL,							/* Redraw cache */
  &_sgi_map_params
};

Display		SgiMapDisplay = &_SgiMapDisplay;


#define RGB_ERASE 0xff000000
#define RGB_BLACK 0xff010000
#define SUBSTITUTE_FOR_BLACK 0xff0000cc

SgiMapColorDef	SgiMapColorTable[] = {

    0,   0,   0, 	/* Black	*/
  255,   0,   0,	/* Red		*/
    0, 255,   0,	/* Green	*/
  255, 255,   0,	/* Yellow	*/
    0,   0, 255,	/* Blue		*/
  255,   0, 255,	/* Magenta	*/
    0, 255, 255,	/* Cyan		*/
  255, 255, 255,	/* White	*/

  0x11, 0x11, 0x11,	/* Grey1	*/
  0x33, 0x33, 0x33,	/* Grey2	*/
  0x55, 0x55, 0x55,	/* Grey3	*/
  0x77, 0x77, 0x77,	/* Grey4	*/
  0x99, 0x99, 0x99,	/* Grey5	*/
  0xbb, 0xbb, 0xbb,	/* Grey6	*/
  0xdd, 0xdd, 0xdd,	/* Grey7	*/
  0xff, 0xff, 0xff,	/* Grey8	*/

  0x00, 0x00, 0x00,   /* TRANSPARENT */

};

SgiMapColorDef	SgiMapInverseTable[] = {

    0,   0,   0, 	/* Black	*/
    0, 255, 255,	/* Cyan		*/
  255,   0, 255,	/* Magenta	*/
    0,   0, 255,	/* Blue		*/
  255, 255,   0,	/* Yellow	*/
    0, 255,   0,	/* Green	*/
  255,   0,   0,	/* Red		*/
  255, 255, 255,	/* White	*/

  0xff, 0xff, 0xff,	/* Grey8	*/
  0xdd, 0xdd, 0xdd,	/* Grey7	*/
  0xbb, 0xbb, 0xbb,	/* Grey6	*/
  0x99, 0x99, 0x99,	/* Grey5	*/
  0x77, 0x77, 0x77,	/* Grey4	*/
  0x55, 0x55, 0x55,	/* Grey3	*/
  0x33, 0x33, 0x33,	/* Grey2	*/
  0x11, 0x11, 0x11,	/* Grey1	*/

  0x00, 0x00, 0x00,   /* TRANSPARENT */

};


SgiMapColorDef	SgiMapGreyTable[] = {

  0x00, 0x00, 0x00,

  0x11, 0x11, 0x11,	/* Grey1	*/
  0x22, 0x22, 0x22,
  0x33, 0x33, 0x33,	/* Grey2	*/
  0x44, 0x44, 0x44,
  0x55, 0x55, 0x55,	/* Grey3	*/
  0x66, 0x66, 0x66,
  0x77, 0x77, 0x77,	/* Grey4	*/
  0x88, 0x88, 0x88,
  0x99, 0x99, 0x99,	/* Grey5	*/
  0xaa, 0xaa, 0xaa,
  0xbb, 0xbb, 0xbb,	/* Grey6	*/
  0xcc, 0xcc, 0xcc,
  0xdd, 0xdd, 0xdd,	/* Grey7	*/
  0xee, 0xee, 0xee,
  0xff, 0xff, 0xff,	/* Grey8	*/

  0x00, 0x00, 0x00,   /* TRANSPARENT */

};

local int sgi_map_default_colors = 17;

#define CUSTOM_MAPS 12
SgiMapColorDef SgiMapCustomTable[CUSTOM_MAPS][256];
int SgiMapCustomColors[256];


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

unsigned short SgiMapPatternTable[] = {
  0xffff,
  0xfff0,
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

} SgiMapDisplayCacheItem;

enum { CODE_CLEAR, CODE_POINT, CODE_MOVE, CODE_DRAW, 
       CODE_ERASE_RECTANGLE, CODE_FILLED_RECTANGLE, CODE_RECTANGLE,
       CODE_CIRCLE, CODE_FILLED_CIRCLE,
       CODE_COLOR, CODE_PATTERN, CODE_STRING,
       CODE_BGNTRACEPOLY, CODE_BGNFILLPOLY,
       CODE_VERTEX, 
       CODE_ENDTRACEPOLY, CODE_ENDFILLPOLY };

local SgiMapDisplayCacheItem root = { CODE_CLEAR, 0, 0, 0, 0, 0 };
local SgiMapDisplayCacheItem *current;
local cache_enable = TRUE;

local fmfonthandle font1, fontx;
local int font_height = 12;

local init_cache( long window ) {
  current = &root;
  root.window = window;
  root.next = 0;
  root.color = BLACK;
}

void SgiMapCacheEnable ( void ) {
  cache_enable = YES;
}

void SgiMapCacheDisable ( void ) {
  cache_enable = NO;
}

local void add_to_cache( long window, int code, 
						 float x1, float y1, float x2, float y2,
						 char *str, unsigned long color ) {

  SgiMapDisplayCacheItem *new;

  if ( !cache_enable ) return;

  if ( code == CODE_CLEAR ) {
    init_cache( window );
  }
  else {
    new = malloc( sizeof( SgiMapDisplayCacheItem ) );
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
} SgiMapRGBColor;

SgiMapDisplayRedraw ( void ) {

  extern int psgl_enabled;

  long width, height;
  long counter = 1;
  float  current_subdivide;

  float vect[2];

  SgiMapDisplayCacheItem *ptr;

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
      color( ptr->color );
      clear();
      break;

    case CODE_MOVE:
      move2( ptr->x1, ptr->y1 );
      break;

    case CODE_POINT:
      pnt2( ptr->x1, ptr->y1 );
      break;

    case CODE_DRAW:
      draw2( ptr->x1, ptr->y1 );
      break;

    case CODE_PATTERN:
      setlinestyle( ptr->color );
      break;

    case CODE_COLOR:
	  color( ptr->color );
      break;

    case CODE_STRING:
      cmov2( ptr->x1, ptr->y1 );
      if ( font1 ) fmprstr( ptr->str );
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
      circi( ptr->x1, ptr->y1, ptr->x2 );
      break;

    case CODE_ERASE_RECTANGLE:
      color( BLACK );
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

  qdevice( F1KEY );
  qdevice( F2KEY );
  qdevice( F3KEY );
  qdevice( F4KEY );
  qdevice( F5KEY );
  qdevice( F6KEY );
  qdevice( F7KEY );
  qdevice( F8KEY );
  qdevice( F9KEY );
  qdevice( F10KEY );
  qdevice( F11KEY );
  qdevice( F12KEY );

  qreset();

}

/***************************************************************************/

local char psroot[256] = "Graphics";
void SgiMapRootPS ( char *root ) {
  strcpy( psroot, root );
}

/***************************************************************************/

void SgiMapHardcopy ( Display display ) {

  char psfile[256];
  int  psfilecnt = 0;

  for ( psfilecnt = 0; psfilecnt < 100; psfilecnt++ ) {
    sprintf( psfile, "%s.%02d.ps", psroot, psfilecnt );
    if ( access( psfile, F_OK ) ) break;
  }
  fprintf( stderr, "Postscript file: %s ... open.\n", psfile );
  fflush( stderr );
  psgl_open( psfile );
  SgiMapDisplayRedraw();
  swapbuffers();
  fprintf( stderr, "Postscript file: %s ... closed.\n", psfile );


}

/***************************************************************************/

void SgiMapHardcopyPict ( Display display ) {

  char psfile[256];
  int  psfilecnt = 0;

  for ( psfilecnt = 0; psfilecnt < 100; psfilecnt++ ) {
    sprintf( psfile, "%s%02d.pct", psroot, psfilecnt );
    if ( access( psfile, F_OK ) ) break;
  }
  fprintf( stderr, "Pict file: %s ... open.\n", psfile );
  fflush( stderr );
  psgl_open_pict( psfile );
  SgiMapDisplayRedraw();
  swapbuffers();
  fprintf( stderr, "Pict file: %s ... closed.\n", psfile );


}

/***************************************************************************/

void	SgiMapInstallMap ( Display display, SgiMapColorDef *map, int n ) {

  int i;
  for ( i = 0; i < n; i++ ) {
	mapcolor( i, 
			  map[i].red,
			  map[i].green,
			  map[i].blue );
  }

}

void SgiMapDefaultColors( Display display ) {
  SgiMapInstallMap( display, 
					SgiMapColorTable, 
					sgi_map_default_colors );
}

void SgiMapCustom( Display display, int table ) {
  SgiMapInstallMap( display, 
					SgiMapCustomTable[table], 
					SgiMapCustomColors[table] );
}

void SgiMapInverseColors( Display display ) {
  SgiMapInstallMap( display, 
					SgiMapInverseTable, 
					sgi_map_default_colors );
}

void SgiMapGreyColors( Display display ) {
  SgiMapInstallMap( display, 
					SgiMapGreyTable, 
					sgi_map_default_colors );
}

void SgiMapWhiteBackground( Display display ) {

  mapcolor( BLACK, 
			SgiMapColorTable[WHITE].red,
			SgiMapColorTable[WHITE].green,
			SgiMapColorTable[WHITE].blue );
  mapcolor( WHITE, 
			SgiMapColorTable[BLACK].red,
			SgiMapColorTable[BLACK].green,
			SgiMapColorTable[BLACK].blue );

}

/***************************************************************************/

void SgiReadMaps( Display display, char *filename ) {

  FILE *fp;

  int i, key, colors, red, green, blue;
  char keyid[256];

  fp = pp_open( filename, "r" );
  if ( !fp ) {
	fprintf( stderr, "Error opening color map file %s.\n", filename );
	exit( -1 );
  }

  while ( 2 == fscanf( fp, "%s %d", keyid, &colors ) ) {

	if ( keyid[0] != 'F' ) {
	  fprintf( stderr, "Error reading map for key %s.\n", keyid );
	  exit( -2 );
	}

	if ( 1 != sscanf( keyid+1, "%d", &key ) ) {
	  fprintf( stderr, "Error reading function key %s.\n", keyid+1 );
	}
	key--;
	fprintf( stderr, "Key: %s %d Colors: %d\n", keyid, key, colors );

	SgiMapCustomColors[key] = colors;
	for ( i = 0; i < colors; i++ ) {
	  fscanf( fp, "%d %d %d", &red, &green, &blue );
	  SgiMapCustomTable[key][i].red = red;
	  SgiMapCustomTable[key][i].green = green;
	  SgiMapCustomTable[key][i].blue = blue;
	  fprintf( stderr, "%d %d: %d %d %d\n", 
			   key, i,
			   SgiMapCustomTable[key][i].red,
			   SgiMapCustomTable[key][i].green,
			   SgiMapCustomTable[key][i].blue );

	}

  }

}

void	SgiMapSetCustomMap ( Display display, int which,
							 SgiMapColorDef *map, int colors ) {

  int i;
  SgiMapCustomColors[which] = colors;
  for ( i = 0; i < colors; i++ ) {
	  SgiMapCustomTable[which][i].red = map[i].red;
	  SgiMapCustomTable[which][i].green = map[i].green;
	  SgiMapCustomTable[which][i].blue = map[i].blue;
  }

}

/***************************************************************************/

void	SgiMapInit ( Display display ) {

  register SgiMapParams	*params = display->parameters;

  long    current_linewidth;
  long    window_width, window_height;
  long    screen_width, screen_height;
  long    screen_width_mm, screen_height_mm;

  long	  width, height;
  double  mag = 1.0;
  char    window_name[256];

  double  pixels_per_mm;

  long preferred_top = 200;
  long preferred_bottom = 735;
  long preferred_left = 200;
  long preferred_right = 1024;

  char *page;

  int i, j, table;
    
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
      sscanf( page, "%lf", &mag );
    }
    else {

      mag = (double) screen_width / (double) width;
      if (( (double) screen_height / (double) height ) < mag ) {
		mag = (double) screen_height / (double) height;
      }
      mag = floor( mag );


    }
    width *= mag;
    height *= mag;
    display->symbol_radius *= mag;
    if ( width > screen_width - 40 ) width -= 40;
    if ( height > screen_height - 40 ) height -= 40;

    fprintf( stderr, "%d %d %f %d %d\n",
			 screen_width, screen_height, mag, width, height );
    preferred_right = screen_width - 20;
    preferred_left = preferred_right - width;
    preferred_top = screen_height - 20;
    preferred_bottom = preferred_top - height;

    prefposition( preferred_left, preferred_right, 
				  preferred_top, preferred_bottom );

    /* Indicate the magnification, if any, in the window name. */
    if ( mag != 1.0 ) sprintf( window_name, "%s X %.0f", params->name, mag );
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
  zbuffer( TRUE );
  doublebuffer();
  gconfig();

  for ( table = 0; table < CUSTOM_MAPS; table++ ) {
	for ( j = 0; j < 256; j++ ) {
	  SgiMapCustomTable[table][j].red = 0;
	  SgiMapCustomTable[table][j].green = 0;
	  SgiMapCustomTable[table][j].blue = 0;
	}
  }
  SgiMapCustomColors[table] = 0;
	  
  SgiMapDefaultColors( display );

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


  SgiMapErase( display );
  SgiMapColor( display, WHITE );
 
  fminit();
  font1 = fmfindfont( "Times-Roman" );
  if ( !font1 ) fprintf( stderr, "Error finding font Times-Roman.\n" );
  else {
    fmfontinfo info;
    fontx = fmscalefont( font1, (double) 12 * mag );
    fmgetfontinfo( fontx, &info );
    /* Knock off 2 pixels to get a tighter line spacing. */
    font_height = info.height - 2;
    fmsetfont( fontx );
  }

  for ( i = 1; i < PATTERNS; i++ ) {
    deflinestyle( i, SgiMapPatternTable[i] );
  }

}

/***************************************************************************/

void SgiMapClose ( Display display ) {

  register SgiMapParams	*params = display->parameters;
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

		if ( Shift ) SgiMapHardcopyPict( display );
		else SgiMapHardcopy( display );
		break;

      case ESCKEY:
		gexit();
		exit( 0 );
		break;

	  case F1KEY:
		fprintf( stderr, "F1\n" );
		SgiMapCustom( display, 0 );
		break;

	  case F2KEY:
		SgiMapCustom( display, 1 );
		break;

	  case F3KEY:
		SgiMapCustom( display, 2 );
		break;

	  case F4KEY:
		SgiMapCustom( display, 3 );
		break;

	  case F5KEY:
		SgiMapCustom( display, 4 );
		break;

	  case F6KEY:
		SgiMapCustom( display, 5 );
		break;

	  case F7KEY:
		SgiMapCustom( display, 6 );
		break;

	  case F8KEY:
		SgiMapCustom( display, 7 );
		break;

	  case F9KEY:
		SgiMapCustom( display, 8 );
		break;

	  case F10KEY:
		SgiMapCustom( display, 9 );
		break;

	  case F11KEY:
		SgiMapCustom( display, 10 );
		break;
		
	  case F12KEY:
		SgiMapDefaultColors( display );
		break;
		
      default:

		/* Redraw the display. */
		SgiMapDisplayRedraw();
		swapbuffers();

		break;

      }
    }
    else {
      SgiMapDisplayRedraw();
      swapbuffers();
    }
  }
}

/***************************************************************************/

int	SgiMapInput( Display display, float *x, float *y ) {

  register SgiMapParams	*params = display->parameters;
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

      if ( Shift ) SgiMapHardcopyPict( display );
      else SgiMapHardcopy( display );
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
      SgiMapDisplayRedraw();
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

#define PEN_RADIUS	1

void	SgiMapPoint ( Display display, float x, float y) {

  register SgiMapParams	*params = (SgiMapParams *)display->parameters;

  winset( params->window );
  pnt2( x, y );
  add_to_cache( params->window, CODE_POINT, x, y, 0, 0, 0, 0 );
	
}

void	SgiMapLine	( Display display, float x1, float y1, float x2, float y2) {

  register SgiMapParams	*params = (SgiMapParams *)display->parameters;

  winset( params->window );
  move2( x1, y1 );
  draw2( x2, y2 );

  add_to_cache( params->window, CODE_MOVE, x1, y1, 0, 0, 0, 0 );
  add_to_cache( params->window, CODE_DRAW, x2, y2, 0, 0, 0, 0 );
	
}

void	SgiMapMoveTo ( Display display, float x1, float y1) {

  register SgiMapParams	*params = (SgiMapParams *)display->parameters;

  winset( params->window );
  move2( x1, y1 );
  add_to_cache( params->window, CODE_MOVE, x1, y1, 0, 0, 0, 0 );

	
}

void	SgiMapLineTo	( Display display, float x2, float y2 ) {

  register SgiMapParams	*params = (SgiMapParams *)display->parameters;

  winset( params->window );
  draw2( x2, y2 );
  add_to_cache( params->window, CODE_DRAW, x2, y2, 0, 0, 0, 0 );

	
}

void	SgiMapText	( Display display, char *string, 
					  float x, float y, double dir ) {
	
  register SgiMapParams	*params = display->parameters;
	
  winset( params->window );
  cmov2( x, y );
  if ( font1 ) fmprstr( string );
  else charstr( string );
  add_to_cache( params->window, CODE_STRING, x, y, 0, 0, string, 0 );
 
}

float	SgiMapTextWidth ( Display display, char *string ) {
	
  if ( font1 ) return( fmgetstrwidth( fontx, string ) );
  else return ( strwidth( string ) );
	
}
	
float	SgiMapTextHeight ( Display display, char *string ) {
	
  return ( font_height );
	
}
	
	
void	SgiMapRectangle	( Display display, 
						  float x1,float y1, float x2, float y2) {
	
  register SgiMapParams	*params = display->parameters;
  winset( params->window );
  recti( x1, y1, x2, y2 );
  add_to_cache( params->window, CODE_RECTANGLE, x1, y1, x2, y2, 0, 0 );


}

void	SgiMapFilledRectangle ( Display display, 
								float x1, float y1, float x2, float y2) {
	
  register SgiMapParams	*params = display->parameters;
  winset( params->window );
  rectf( x1, y1, x2, y2 );
  add_to_cache( params->window, CODE_FILLED_RECTANGLE, x1, y1, x2, y2, 0, 0 );

}

void	SgiMapCircle	(Display display, float x, float y, float radius ) {

  register SgiMapParams	*params = (SgiMapParams *)display->parameters;
  winset( params->window );
  circi( x, y, radius );
  add_to_cache( params->window, CODE_CIRCLE, x, y, radius, 0, 0, 0 );
		
}

void	SgiMapFilledCircle	(Display display, float x, float y, float radius )

{
  register SgiMapParams	*params = (SgiMapParams *)display->parameters;
  winset( params->window );
  circf( x, y, radius );
  add_to_cache( params->window, CODE_FILLED_CIRCLE, x, y, radius, 0, 0, 0 );
}

void	SgiMapEraseRectangle ( Display display, 
							   float x1, float y1, float x2, float y2) {

  register SgiMapParams	*params = (SgiMapParams *)display->parameters;

  winset( params->window );
  color( BLACK );
  rectf( x1, y1, x2, y2 );
  add_to_cache( params->window, CODE_ERASE_RECTANGLE, x1, y1, x2, y2, 0, 0 );

}

void	SgiMapErase ( Display display) {

  register SgiMapParams	*params = display->parameters;

  winset( params->window );
  color( BLACK );
  clear();
  zclear();
  add_to_cache( params->window, CODE_CLEAR, 0, 0, 0, 0, 0, 0 );

}

void	SgiMapAlu	( Display display, int alu) {

  register SgiMapParams	*params = display->parameters;
  winset( params->window );

}

void	SgiMapLineStyle ( Display display, int style ) {
	
  register SgiMapParams	*params = display->parameters;
  winset( params->window );

  if ( display->bw ) SgiMapLinePattern( display, StyleToPattern( style ) );
  else SgiMapColor( display, StyleToColor( style ) );
 
}

void	SgiMapLinePattern	( Display display, int pattern ) {
	
  register SgiMapParams	*params = display->parameters;
  winset( params->window );

  setlinestyle( pattern );
  add_to_cache( params->window, CODE_PATTERN, 0, 0, 0, 0, 0, pattern );

}

void	SgiMapColor ( Display display, int index ) {
	
  register SgiMapParams	*params = (SgiMapParams *)display->parameters;
  winset( params->window );

  color( index );
  add_to_cache( params->window, CODE_COLOR, 0, 0, 0, 0,	0, index );
 
}

void	SgiMapPenSize ( Display display, float size) {
	
  register SgiMapParams	*params = (SgiMapParams *)display->parameters;
  winset( params->window );

  linewidthf( size );

}

void	SgiMapStartPolygon ( Display display ) {

  register SgiMapParams	*params = display->parameters;
  winset( params->window );

  params->vertex_count = 0;

}
void	SgiMapAddVertex ( Display display, float x, float y ) {

  register SgiMapParams	*params = display->parameters;
  winset( params->window );

  if ( params->vertex_count >= SGI_MAX_POLY_POINTS ) return;
  params->vertex[ params->vertex_count ].x = x;
  params->vertex[ params->vertex_count ].y = y;
  params->vertex_count++;


}

void	SgiMapOutlinePolygon ( Display display ) {

  float vect[2];
  int i;

  register SgiMapParams	*params = display->parameters;
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
void	SgiMapFillPolygon ( Display display ) {

  float vect[2];
  int i;

  register SgiMapParams	*params = display->parameters;
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


