/*****************************************************************************/
/*                                                                           */
/*                                  Layouts                                  */
/*                                                                           */
/*****************************************************************************/
// Defines that increase the amount of info in the memory leak report.

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "Layouts.h"

Layout _layout_destroy_list = 0;

/*****************************************************************************/

void LayoutConfigure( Layout layout, int rows, int columns ) {

  int i;
	
  layout->rows = rows;
  layout->columns = columns;

  if ( rows * columns > layout->n_views ) {

    layout->views = (View *) realloc( layout->views, rows * columns * sizeof(View));

    if ( !layout->views ) {
      fprintf(stderr, "LayoutCreate - malloc error for View array.\n" );
      exit( -1 );
    }

    for ( i = layout->n_views; i < rows * columns; i++ ) {
      layout->views[ i ] = CreateView( layout->display );
    }
    layout->n_views = rows * columns;

  }
    
  LayoutSetDisplayEdgesRelative( layout, layout->left, layout->bottom, 
				 layout->right, layout->top );

}

void DestroyLayouts ( void ) {
	Layout layout = _layout_destroy_list;
	Layout hold;
	while ( layout ) {
		free( layout->views );
		hold = layout;
		layout = layout->next;
		free( hold );
	}
}


/*****************************************************************************/

void LayoutDescribe ( Layout layout ) {

  fprintf( stderr, "Layout: %8p  Rows: %d  Columns: %d\n", 
	   layout, layout->rows, layout->columns );
  fprintf( stderr, "             Left: %d Right: %d  Top: %d  Bottom: %d\n", 
	   layout->display_left, layout->display_right, 
	   layout->display_top, layout->display_bottom );

}

/*****************************************************************************/

void LayoutSetSpacing( Layout layout, double x_space, double y_space ) {
  
  layout->x_space = x_space;
  layout->y_space = y_space;

  LayoutSetDisplayEdgesRelative( layout, layout->left, layout->bottom, 
				 layout->right, layout->top );

}

/*****************************************************************************/

Layout CreateLayout( Display display, int rows, int columns ) {

  Layout layout;

  layout = (Layout) malloc( sizeof(struct _layout) );
  if ( !layout ) {
    fprintf(stderr, "LayoutCreate - malloc error for base structure.\n" );
    exit( -1 );
  }

  layout->display = display;
  layout->n_views = 0;
  layout->rows = 0;
  layout->columns = 0;
  layout->views = NULL;

  layout->left = 0.01;
  layout->right = 0.98;
  layout->bottom = 0.01;
  layout->top = 0.98;

  layout->x_space = 0.005;
  layout->y_space = 0.005;

  LayoutConfigure( layout, rows, columns );
  
  layout->next = _layout_destroy_list;
  _layout_destroy_list = layout;
 
  return( layout );

}


/*****************************************************************************/

View LayoutView( Layout layout, int row, int column ) {

  if ( row >= layout->rows || column > layout->columns ) {
    fprintf( stderr, "Invalid layout coordinates: %d %d (%d %d)\n",
	     row, column, layout->rows, layout->columns );
    row = 0;
    column = 0;
  }

    return( layout->views[ (row) * layout->columns + (column)] );

}


/*****************************************************************************/

View LayoutViewN( Layout layout, int n ) {

  if ( n >= layout->rows * layout->columns ) {
    fprintf( stderr, "Invalid layout number: %d (%d)\n",
	     n, layout->rows * layout->columns );
    n = 0;
  }

  return( layout->views[ n ] );

}

/*****************************************************************************/

View LayoutViewNT( Layout layout, int n ) {

  int row, column;

  if ( n >= layout->rows * layout->columns ) {
    fprintf( stderr, "Invalid layout number: %d (%d)\n",
	     n, layout->rows * layout->columns );
    n = 0;
  }

  column = n / layout->rows;
  row = n % layout->rows;
  return( LayoutView( layout, row, column ) );

}

/*****************************************************************************/

void LayoutSetDisplayEdgesRelative( Layout layout, 
				    double left, double bottom, 
				    double right, double top ) {

  int row, column;
  double x, y;

  View view;

  double width = (right - left) / (double) layout->columns;
  double height = (top - bottom) / (double) layout->rows;
  double x_space = layout->x_space;
  double y_space = layout->y_space;

  for ( row = 0; row < layout->rows; row++ ) {
    for ( column = 0; column < layout->columns; column++ ) {

      x = left + column * width;
      y = top - row * height;
      ViewSetDisplayEdgesRelative( LayoutView( layout, row, column ),
				   x + x_space, 
				   y - height + y_space, 
				   x + width - x_space, 
				   y - y_space );



    }
  }

  view = LayoutView( layout, 0, 0 );
  layout->display_left = view->display_left;
  layout->display_top = view->display_top;

  view = LayoutView( layout, layout->rows - 1, layout->columns - 1 );
  layout->display_right = view->display_right;
  layout->display_bottom = view->display_bottom;

}



/*****************************************************************************/

int LayoutInput( Layout layout, View *view, int *row, int *column ) {

  float ix, iy;
  int   event;

  event = DisplayInput( layout->display, &ix, &iy );

  for ( *row = 0; *row < layout->rows; (*row)++ ) {
    for ( *column = 0; *column < layout->columns; (*column)++ ) {

      *view = LayoutView( layout, *row, *column );
      if ( ViewDisplayPointInside( *view, ix, iy ) ) {

	return( event );

      }

    }
  }

  *view = 0;
  return( event );
 
}

/*****************************************************************************/

int LayoutProcessInput( Layout layout, int display_x, int display_y,
			int *row, int *column ) {

  View view;
  int lrow, lcolumn;

  for ( lrow = 0; lrow < layout->rows; lrow++ ) {
    for ( lcolumn = 0; lcolumn < layout->columns; lcolumn++ ) {

      view = LayoutView( layout, lrow, lcolumn );
      if ( ViewDisplayPointInside( view, display_x, display_y ) ) {

	*row = lrow;
	*column = lcolumn;
	return( TRUE );

      }

    }
  }

  return( FALSE );
 
}

/***************************************************************************/

void LayoutTitle (Layout layout, char *string, int x, int y, double dir) {

  float	 x_sense, y_sense, x_offset;

  Display display = layout->display;
  
  char   temp[1024], *line;
  unsigned int    lines = 1, i;
  int    right_justify = FALSE, center_justify = FALSE;
  
  /* Handle carriage returns. */
  for ( i = 0; i < strlen( string ); i++ ) {
    if ( string[i] == '\n' ) lines++;
  }
  
  x_sense = layout->display_right > layout->display_left ? 1 : -1;
  y_sense = layout->display_bottom > layout->display_top ? 1 : -1;
  
  /*
  A value less than 0 for x or y implies a keyword.
  */
  if (x < 0) {
    
    switch ( (int) x ) {
      
    case (INSIDE_LEFT):
      x = layout->display_left;
      break;
      
    case (OUTSIDE_LEFT):
      x = layout->display_left - x_sense;
      right_justify = TRUE;
      /*	x_sense * TextWidth(display, string); */
      break;
      
    case (INSIDE_RIGHT):
      x = layout->display_right - 1;
      right_justify = TRUE;
      /*	x_sense * TextWidth(display, string); */
      break;
      
    case (OUTSIDE_RIGHT):
      x = layout->display_right;
      break;
      
    case (CENTER):
      x = ( layout->display_left + layout->display_right 
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
      y = layout->display_bottom -
        y_sense * ( lines - 1) * TextHeight(display, string);
      break;
      
      
    case (OUTSIDE_BOTTOM):
      y = layout->display_bottom + 
        y_sense * ( TextHeight(display, string) - 2 );
      break;
      
    case (INSIDE_TOP):
      y = layout->display_top +
        y_sense * TextHeight(display, string);
      break;
      
    case (OUTSIDE_TOP):
      y = layout->display_top;
      break;
      
    case (CENTER):
      y = ( layout->display_bottom + layout->display_top 
        + y_sense * lines * TextHeight(display, string)) / 2;
      break;
      
    default:
      fprintf(stderr, "Unknown title y alignment code: %d\n", y);
      break;
      
    }
  }
  
  strcpy( temp, string );
  line = strtok( temp, "\n" );
  while ( line ) {
    if ( right_justify ) x_offset = - x_sense * TextWidth( display, line );
    else if ( center_justify ) x_offset = ( - x_sense * TextWidth( display, line )) / 2.0;
    else x_offset = 0;
    Text( display, line, x + x_offset, y, dir);
    y += y_sense * TextHeight(display, line);
    line = strtok( 0, "\n" );
  }
}

  
  
  
#if 0
  
  int x_sense, y_sense;

  x_sense = layout->display_right > layout->display_left ? 1 : -1;
  y_sense = layout->display_bottom > layout->display_top ? 1 : -1;

  /*
   *    A value less than 0 for x or y implies a keyword.
   */
  switch (x) {

  case (INSIDE_LEFT):
    x = layout->display_left + x_sense;
    break;

  case (OUTSIDE_LEFT):
    x = layout->display_left - 
      x_sense * ( TextWidth(layout->display, string) + 1 );
    break;

  case (INSIDE_RIGHT):
    x = layout->display_right - 
      x_sense * (TextWidth(layout->display, string) + 1 );
    break;
			
  case (OUTSIDE_RIGHT):
    x = layout->display_right + x_sense;
    break;

  case (CENTER):
    x = ( layout->display_left + layout->display_right 
	  - x_sense * TextWidth(layout->display, string)) / 2.0;
    break;

  default:
    x = layout->display_top + x_sense * x;
    break;

  }


  switch (y) {

  case (INSIDE_BOTTOM):
    y = layout->display_bottom - y_sense;
    break;

  case (OUTSIDE_BOTTOM):
    y = layout->display_bottom + 
      y_sense * ( TextHeight(layout->display, string) + 1 );
    break;

  case (INSIDE_TOP):
    y = layout->display_top +
      y_sense * ( TextHeight(layout->display, string) + 1 );
    break;
			
  case (OUTSIDE_TOP):
    y = layout->display_top -
      y_sense * ( 0.3 * TextHeight(layout->display, string) + 1 );
    break;

  case (CENTER):
    y = ( layout->display_bottom + layout->display_top 
	  + y_sense 
	  * TextHeight(layout->display, string)) / 2.0;
    break;

  default:
    y = layout->display_top + y_sense * y;
    break;

  }

  Text( layout->display, string, (float) x, (float) y, dir);

}

#endif


/***************************************************************************/

void LayoutBox (Layout layout ) {

  Line( layout->display, layout->display_left, layout->display_top,
	                 layout->display_right, layout->display_top );

  Line( layout->display, layout->display_right, layout->display_top,
	                 layout->display_right, layout->display_bottom );

  Line( layout->display, layout->display_right, layout->display_bottom,
	                 layout->display_left, layout->display_bottom );

  Line( layout->display, layout->display_left, layout->display_bottom,
	                 layout->display_left, layout->display_top );

}




