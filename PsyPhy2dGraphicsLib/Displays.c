/****************************************************************************/
/*                                                                          */
/*                             Displays.c                                   */
/*                                                                          */
/****************************************************************************/

/*    (c) Copyright 1994, 1995 - PsyPhy Consulting. All Rights Reserved.    */

/*
 * Define device independent displays.
 * This file contains the routines common to all Displays.
 */

// Disable warnings about unsafe functions.
// We use the 'unsafe' versions to maintain source-code compatibility with Visual C++ 6
#define _CRT_SECURE_NO_WARNINGS

// Extra info about memory leaks.
#define _CRTDBG_MAP_ALLOC

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <crtdbg.h>
#include <windows.h>

#include "useful.h"

#include "Graphics.h"
#include "Displays.h"

#pragma warning(disable: 4113)
#pragma warning(disable: 4047)
#pragma warning(disable: 4133)

/****************************************************************************/

/*
 * This provide a dummy display to substitute when we don't have
 * a hardcopy device.
 */

void    _null_proc(); 
int	_null_int_proc(); 

float	_null_float_proc();

double	_null_double_proc();


struct _display _NullDisplay = {
  "NULL Display",
    0, 100, 0, 100,
    _null_proc, _null_proc, _null_proc, _null_proc,
    _null_proc, _null_float_proc, _null_float_proc,
    _null_proc, _null_proc, _null_proc, 
    _null_proc, _null_proc, _null_proc, _null_proc,
    _null_proc, _null_proc, _null_proc, _null_proc, _null_proc,
    _null_proc, _null_proc, _null_proc, _null_int_proc,
    0, 0, 0, 0, 2, -1, -1, 0, 0, 0};
  
Display NullDisplay = &_NullDisplay;

void 	_null_proc () {}
int 	_null_int_proc () { return( 0 ); }
float	_null_float_proc () { return( (float) 0.0 ); }

double	_null_double_proc () { return( 0.0 ); }


/****************************************************************************/

/* Map line styles to colors. */

int	style_to_color[STYLES] = {BLACK, GREEN, RED, BLUE, 
MAGENTA, YELLOW, CYAN, BLACK};

/* Map line styles to patterns. */

int	style_to_pattern[STYLES] = {SOLID, SOLID, DASH, DOT, 
DASH_DOT, DASH_DOT_DOT, 
LONG_DASH, CENTER_DASH};

/* Select colors in the same order as GlViews. */

int     color_select_table[6] = {RED, GREEN, BLUE, MAGENTA, CYAN, YELLOW};

/****************************************************************************/

/*
Select default display at run time, if the machine has multiple
displays and you want to be able to change on the run.
*/

extern Display	OglDisplay;
Display DefaultDisplay ( void ) {
  return( OglDisplay );  
}

/****************************************************************************/

Display _display_destroy_list = 0;
void DestroyDisplays ( void ) {
	Display display = _display_destroy_list;
	Display hold;
	while ( display ) {
		(display->close)(display);
		hold = display;
		display = display->next;
		free( hold );
	}
}

/****************************************************************************/

Display CreateDisplay( Display model ) {

	Display display;

	display = malloc( sizeof( model ) );
	if ( ! display ) {
		MessageBox( NULL, "Unable to create display.", "Display Error", MB_OK );
		exit( -1 );
	}
	memcpy( display, model, sizeof( *display ));
	display->next = _display_destroy_list;
	_display_destroy_list = display;

	return( display );

}

/****************************************************************************/

void  DisplayDescribe (Display display) {
  
  fprintf (stderr, "Display: %lx %s\n", (long) display, display->name);
  fprintf (stderr, "Left: %3f  Top: %3f  Right: %f  Bottom: %f\n",
    display->left, display->top, display->right, display->bottom);
  
}

/***************************************************************************/

void DisplaySetDefaults( Display display ) {

  Erase( display );
  Color( display, BLACK );
  Pen( display, 1 );
  Alu( display, SET );
  LinePattern( display, SOLID );

}

/***************************************************************************/

void  DisplayInit( Display display ) {
  (*(display->init))( display );
  DisplaySetDefaults( display );
}

/***************************************************************************/

void  DisplayActivate( Display display ) {
  (*(display->activate))( display );
}

/***************************************************************************/


void DisplayLabel (Display display, char *string, 
                   float x, float y, double dir, 
                   ViewTxtAlign x_justify, ViewTxtAlign y_justify) {
  
  float		x_offset=0, y_offset=0, x_sense, y_sense;
  
  char   *start, *end, temp[1024];
  unsigned int    lines = 1, i;
  int    right_justify = FALSE, center_justify = FALSE;

  x_sense = display->right > display->left ? 1 : -1;
  y_sense = display->bottom > display->top ? 1 : -1;
  
  /* Handle carriage returns. */
  for ( i = 0; i < strlen( string ); i++ ) {
    if ( string[i] == '\n' ) lines++;
  }
  
  switch (x_justify) {
    
  case (LEFT_JUSTIFY):
    x_offset = 0;
    break;
    
  case (RIGHT_JUSTIFY):
    x_offset = - x_sense * TextWidth(display, string);
    right_justify = TRUE;
    break;
    
  case (CENTER_JUSTIFY):
    x_offset = - x_sense * TextWidth(display, string) / 2.0;
    center_justify = TRUE;
    break;
    
  default:
    fprintf(stderr, "Unknown X alignment code: %d\n", x_justify);
    break;
    
  }
  
  switch (y_justify) {
    
  case (BOTTOM_JUSTIFY):
    y_offset = 0;
    break;
    
  case (TOP_JUSTIFY):
    y_offset = y_sense * TextHeight(display, string);
    break;
    
  case (CENTER_JUSTIFY):
    y_offset = y_sense * lines * TextHeight(display, string) / 2;
    break;
    
  default:
    fprintf(stderr, "Unknown Y alignment code: %d\n", y_justify);
    break;
    
  }
  
  strncpy( temp, string, sizeof( temp ) );
  start = end = temp;
  while ( *end ) {
    if ( *end == '\n' ) {
      if ( right_justify ) x_offset = - x_sense * TextWidth( display, start );
      else if ( center_justify ) x_offset = ( - x_sense * TextWidth( display, start )) / 2.0;
      else x_offset = 0;
      *end = 0;
      Text( display, start, x + x_offset, y + y_offset, dir);
      y_offset += y_sense * TextHeight(display, start);
      start = end + 1;
      end = start;
    }
    else end++;
  }
  if ( right_justify ) x_offset = - x_sense * TextWidth( display, start );
  else if ( center_justify ) x_offset = ( - x_sense * TextWidth( display, start )) / 2.0;
  else x_offset = 0;
  Text( display, start, x + x_offset, y + y_offset, dir);
  
}

/***************************************************************************/

void DisplayTitle (Display display, char *string, float x, float y, double dir) {
  
  float	 x_sense, y_sense, x_offset;
  
  char   temp[1024], *start, *end;
  unsigned int    lines = 1, i;
  int    right_justify = FALSE, center_justify = FALSE;
  
  /* Handle carriage returns. */
  for ( i = 0; i < strlen( string ); i++ ) {
    if ( string[i] == '\n' ) lines++;
  }
  
  x_sense = display->right > display->left ? 1 : -1;
  y_sense = display->bottom > display->top ? 1 : -1;
  
  /*
  A value less than 0 for x or y implies a keyword.
  */
  if (x < 0) {
    
    switch ( (int) x ) {
      
    case (INSIDE_LEFT):
      x = display->left;
      break;
      
    case (OUTSIDE_LEFT):
      x = display->left - x_sense;
      right_justify = TRUE;
      /*	x_sense * TextWidth(display, string); */
      break;
      
    case (INSIDE_RIGHT):
      x = display->right - 1;
      right_justify = TRUE;
      /*	x_sense * TextWidth(display, string); */
      break;
      
    case (OUTSIDE_RIGHT):
      x = display->right;
      break;
      
    case (CENTER):
      x = ( display->left + display->right 
        - x_sense * TextWidth(display, string)) / 2;
      
      center_justify = TRUE;
      break;
      
    default:
      fprintf(stderr, "Unknown title x alignment code: %d\n", x);
      break;
      
    }
  }
  
  if (y < 0) {
    
    switch ( (int) y ) {
      
    case (INSIDE_BOTTOM):
      y = display->bottom -
        y_sense * ( lines - 1) * TextHeight(display, string);
      break;
      
      
    case (OUTSIDE_BOTTOM):
      y = display->bottom + 
        y_sense * ( TextHeight(display, string) - 2 );
      break;
      
    case (INSIDE_TOP):
      y = display->top +
        y_sense * TextHeight(display, string);
      break;
      
    case (OUTSIDE_TOP):
      y = display->top;
      break;
      
    case (CENTER):
      y = ( display->bottom + display->top 
        + y_sense * lines * TextHeight(display, string)) / 2;
      break;
      
    default:
      fprintf(stderr, "Unknown title y alignment code: %d\n", y);
      break;
      
    }
  }
  
  strncpy( temp, string, sizeof( temp ) );
  start = end = temp;
  while ( *end ) {
    if ( *end == '\n' ) {
      if ( right_justify ) x_offset = - x_sense * TextWidth( display, start );
      else if ( center_justify ) x_offset = ( - x_sense * TextWidth( display, start )) / 2.0;
      else x_offset = 0;
      *end = 0;
      Text( display, start, x + x_offset, y, dir);
      y += y_sense * TextHeight(display, start);
      start = end + 1;
      end = start;
    }
    else end++;
  }
  if ( right_justify ) x_offset = - x_sense * TextWidth( display, start );
  else if ( center_justify ) x_offset = ( - x_sense * TextWidth( display, start )) / 2.0;
  else x_offset = 0;
  Text( display, start, x + x_offset, y, dir);

}

/***************************************************************************/

void DisplaySymbol( Display display, float x, float y, int symbol ) {
  
  switch ( symbol ) {
    
  case SYMBOL_POINT:
    Point( display, x, y );
    break;
    
  case SYMBOL_OPEN_SQUARE:
    
    Moveto( display, x - display->symbol_radius, y - display->symbol_radius );
    Lineto( display, x + display->symbol_radius, y - display->symbol_radius );
    Lineto( display, x + display->symbol_radius, y + display->symbol_radius );
    Lineto( display, x - display->symbol_radius, y + display->symbol_radius );
    Lineto( display, x - display->symbol_radius, y - display->symbol_radius );
    
    break;
    
  case SYMBOL_FILLED_CIRCLE:
    
    FilledCircle( display, x, y, display->symbol_radius );
				
    break;
    
  case SYMBOL_FILLED_SQUARE:
    
    FilledRectangle( display, x - display->symbol_radius, y - display->symbol_radius,
      x + display->symbol_radius, y + display->symbol_radius );
				
    break;
    
  case SYMBOL_X:
    
    Line( display, x - display->symbol_radius, y - display->symbol_radius, 
      x + display->symbol_radius, y + display->symbol_radius );
    Line( display, x - display->symbol_radius, y + display->symbol_radius, 
      x + display->symbol_radius, y - display->symbol_radius );
    
    break;
    
  case SYMBOL_CROSS:
    
    Line( display, x, y - display->symbol_radius, x, y + display->symbol_radius );
    Line( display, x - display->symbol_radius, y, x + display->symbol_radius, y );
    
    break;
    
  case SYMBOL_STAR:
    
    Line( display, x - display->symbol_radius, y - display->symbol_radius, 
      x + display->symbol_radius, y + display->symbol_radius );
    Line( display, x - display->symbol_radius, y + display->symbol_radius, 
      x + display->symbol_radius, y - display->symbol_radius );
    Line( display, x, y - display->symbol_radius, x, y + display->symbol_radius );
    Line( display, x - display->symbol_radius, y, x + display->symbol_radius, y );
    
    break;

  case SYMBOL_OPEN_CIRCLE:
  default:
    
    Circle( display, x, y, display->symbol_radius );
				
    break;
    
  }
  
  
  
}

/***************************************************************************/

void DisplayBox (Display display) {
  
  Moveto( display, display->left, display->bottom );
  Lineto( display, display->left, display->top );
  Lineto( display, display->right, display->top );
  Lineto( display, display->right, display->bottom );
  Lineto( display, display->left, display->bottom );
  
} 

/***************************************************************************/

void DisplayArrow (Display display, 
                   float from_x, float from_y, float to_x, float to_y ) {
  
  double delta_x, delta_y, x, y, dx, dy, orientation, s, c, size;
  
  delta_x = to_x - from_x;
  delta_y = to_y - from_y;
  
  Line( display, from_x, from_y, to_x, to_y );
  
  if ( sqrt( delta_x * delta_x + delta_y * delta_y ) 
	   < display->symbol_radius ){
    return;
  }
  
  orientation = atan2( delta_y, delta_x );
  s = sin( orientation );
  c = cos( orientation );
  size = display->symbol_radius;
  
  StartPolygon( display );
  
  AddVertex( display, to_x, to_y );
  
  dx = - 2.0 * size;
  dy = size;
  x = to_x + c * dx - s * dy;
  y = to_y + s * dx + c * dy;
  AddVertex( display, x, y );
  
  
  dx = - size;
  dy = 0.0;
  x = to_x + c * dx - s * dy;
  y = to_y + s * dx + c * dy;
  AddVertex( display, x, y );
  
  dx = - 2.0 * size;
  dy = - size;
  x = to_x + c * dx - s * dy;
  y = to_y + s * dx + c * dy;
  AddVertex( display, x, y );
  
  AddVertex( display, to_x, to_y );
  
  OutlinePolygon( display );
  FillPolygon( display );
  
}

/***************************************************************************/

void DisplaySetSizeInches (Display display, double width, double height ) {
  
  display->desired_width = width;
  display->desired_height = height;
  
}

void DisplaySetSizeCM (Display display, double width, double height ) {
  
  display->desired_width = width / 2.54;
  display->desired_height = height / 2.54;
  
}

void DisplaySetSizePixels (Display display, int width, int height ) {
  
  display->desired_width = width;
  display->desired_height = height;
  
}

void DisplaySetScreenPosition (Display display, int left, int top ) {
  
  display->desired_left = left;
  display->desired_top = top;
  
}

/***************************************************************************/

void DisplaySetBlackAndWhite ( Display display ) {
  
  display->bw = TRUE;
  
}

/***************************************************************************/

void  DisplaySwap( Display display ) {
  
  (*(display->swap))( display );
  
}

/***************************************************************************/

int  DisplayInput( Display display, float *x, float *y ) {
  
  return( (*(display->input))( display, x, y ) );
  
}

/***************************************************************************/

void DisplaySetName( Display display, char *name ) {
  
  strncpy( display->name, name, sizeof( display->name ) );
  
}

/***************************************************************************/

#define peek( item ) ( (item) ? (item->token) : null_token )

// Enable or disable the redraw cache.
void DisplayEnableCache( Display display ) { display->cache_active = YES; }
void DisplayDisableCache( Display display ) { display->cache_active = NO; }

// Insert a graphics acton at the end of the linked list.
DisplayCacheItem *DisplayInsertCacheItem( Display display ) {

	DisplayCacheItem *item = malloc( sizeof( DisplayCacheItem ) ); 
	display->last_cache->next = item;
	item->previous = display->last_cache;
	item->next = NULL;
	display->last_cache = display->last_cache->next;

	return( item );

}

// Reset the cache. 
void DisplayInitCache( Display display ) {

	// Free the memory used to create the current list.
	if ( display->cache ) DisplayFreeCache( display );

	// Prepare for a new list by creating the initial entry.
	display->last_cache = display->cache = malloc( sizeof( DisplayCacheItem ) );
	display->cache->next = NULL;
	display->cache->previous = NULL;
	display->cache->token = null_token;
	display->cache_active = YES;

}

// Free all the memory currently used by the cache.
// NB The cache cannot be reused as is after this routine.
// To empty and restart the cache, use DisplayInitCache.
void DisplayFreeCache ( Display display ) {
  
  DisplayCacheItem *item, *tofree;
  display->cache_active = NO;

  item = display->cache;
  while( item ) {
	  tofree = item;
	  item = item->next;
	  free( tofree );
  }
  display->cache = display->last_cache = NULL;

}


void DisplayWalkCache ( Display input, Display output ) {
  
  int count = 0;
  int hold;
  int trace_on = NO;
  int trace_count;

  DisplayCacheItem *item;

  hold = input->cache_active;
  input->cache_active = NO;

  DisplaySetDefaults( output );

  for ( item = input->cache, count = 0; item; item = item->next, count++ ) {

    switch ( item->token ) {

    case point_token:
      Point( output, item->param.point.x, item->param.point.y );
      if ( peek( item->next ) == lineto_token ) {
        trace_count = 0;
        StartTrace( output, item->param.point.x, item->param.point.y );
        trace_on = YES;
      }
      break;

    case moveto_token:
      if ( peek( item->next ) == lineto_token ) {
        StartTrace( output, item->param.point.x, item->param.point.y );
        trace_on = YES;
      }
      else {
        Moveto( output, item->param.point.x, item->param.point.y );
        trace_on = NO;
      }
      break;

    case lineto_token:
      if ( trace_on ) {
        if ( peek( item->next ) == lineto_token ) {
          ContinueTrace( output, item->param.point.x, item->param.point.y );
        }
        else {
          EndTrace( output, item->param.point.x, item->param.point.y );
          trace_on = NO;
        }
      }
      else Lineto( output, item->param.point.x, item->param.point.y );
      break;

    case line_token:
      Line( output, item->param.line.x1, item->param.line.y1, 
        item->param.line.x2, item->param.line.y2 );
      break;

    case rectangle_token:
      Rectangle( output, item->param.line.x1, item->param.line.y1, 
        item->param.line.x2, item->param.line.y2 );
      break;

    case filled_rectangle_token:
      FilledRectangle( output, item->param.line.x1, item->param.line.y1, 
        item->param.line.x2, item->param.line.y2 );
      break;

    case erase_rectangle_token:
      EraseRectangle( output, item->param.line.x1, item->param.line.y1, 
        item->param.line.x2, item->param.line.y2 );
      break;

    case circle_token:
      Circle( output, item->param.circle.x, item->param.circle.y, item->param.circle.radius );
      break;

    case filled_circle_token:
      FilledCircle( output, item->param.circle.x, item->param.circle.y, item->param.circle.radius );
      break;

    case start_polygon_token:
      StartPolygon( output );
      break;

    case add_vertex_token:
      AddVertex( output, item->param.point.x, item->param.point.y );
      break;

    case outline_polygon_token:
      OutlinePolygon( output );
      break;

    case fill_polygon_token:
      FillPolygon( output );
      break;

    case color_token:
      Color( output, item->param.color );
      break;

    case rgb_token:
      ColorRGB( output, item->param.rgb.r, item->param.rgb.g, item->param.rgb.b );
      break;

    case pattern_token:
      LinePattern( output, item->param.pattern );
      break;

    case pen_token:
      Pen( output, item->param.pen );
      break;

    case alu_token:
      Alu( output, item->param.alu );
      break;

    case text_token:
      Text( output, item->param.text.string,
        item->param.text.x, item->param.text.y, item->param.text.dir );
      break;
    
    case null_token:
      break;

    default:
      fprintf( stderr, "Item %d  Unrecognized token: %d\n", count, item->token );
      break;

    }
    
  }
  input->cache_active = hold;

}

