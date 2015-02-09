/***************************************************************************/
/*                                                                         */
/*                             Arrayplots.c                                */
/*                                                                         */
/***************************************************************************/

/*    (c) Copyright 1994, 1995 - PsyPhy Consulting. All Rights Reserved.    */

// Disable warnings about unsafe functions.
// We use the 'unsafe' versions to maintain source-code compatibility with Visual C++ 6
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include "useful.h"
#include "Displays.h"
#include "Views.h"
#include "Graphics.h"

/***************************************************************************/

/*
	Plot a series of numbers versus the array index.  We have to do a lot of
	stuff to allow variable record lengths - size is the number of bytes
	between successive numbers - best computed with the sizeof() macro.
*/


/***************************************************************************/

/*
 * If the arrays you are working with are very fine grain, plotting
 * every point can be very memory consuming (each point gets put in a 
 * cache for screen refesh. This allows you to plot every N points
 * to reduce the number of points actually plotted.
 *
 * Not all array plotting routines take it into account yet.
 */

local int view_array_step = 1;

void ViewsSetArrayStep( int N ) {

  if ( N < 1 ) N = 1;
  view_array_step = N;

}


/***************************************************************************/

void ViewPlotDoubles ( View view, double *array, 
		       unsigned start, unsigned end, unsigned step, unsigned size ) {
  register unsigned i;
  register double	*pt;

  pt = (double *)(((char *) array) + start * size);
  ViewMoveTo(view, (double) start,  *pt);

  for (i = start + 1; i <= end; i += step ) {

    pt = (double *)(((char *) array) + i * size);
    ViewLineTo(view, (double) i, *pt);
	
  }
}

/***************************************************************************/

void ViewPlotClippedDoubles ( View view, double *array, 
		       unsigned start, unsigned end, unsigned size, double NA ) {
  register unsigned i;
  register double	*pt;
  double y;
  int penon = NO;

  for (i = start; i <= end; i += view_array_step ) {
	  pt = (double *)(((char *) array) + i * size);
	  if ( *pt == NA ) penon = NO;
	  else {
		if ( *pt > view->user_top ) y = view->user_top;
		else if ( *pt < view->user_bottom ) y = view->user_bottom;
		else y = *pt;
		if ( penon == YES ) ViewLineTo( view, (double) i, y );
		else ViewMoveTo( view, (double) i, y );
		penon = YES;
	 }
  }
}

/***************************************************************************/

void ViewPlotAvailableDoubles ( View view, double *array, 
				unsigned start, unsigned end, 
				unsigned size, double NA ) {
  register unsigned i;
  register double	*pt;
  int penon = NO;

  for (i = start; i <= end; i++) {

    pt = (double *)(((char *) array) + i * size);
    if ( *pt != NA ) {
      if ( penon == YES ) ViewLineTo( view, (double) i, *pt);
      else ViewMoveTo( view, (double) i, *pt );
      penon = YES;
    }
    else penon = NO;

  }
}

/***************************************************************************/

void ViewPointPlotAvailableDoubles ( View view, double *array, 
				unsigned start, unsigned end, 
				unsigned size, double NA ) {
  register unsigned i;
  register double	*pt;

  for (i = start; i <= end; i++) {

    pt = (double *)(((char *) array) + i * size);
    if ( *pt != NA ) ViewPoint( view, (double) i, *pt );

  }
}

/***************************************************************************/

void ViewPlotFloats (View view, float *array, 
		     unsigned start, unsigned end, unsigned size ) {
					
  register unsigned i;
  register float	*pt;

  pt = (float *)(((char *) array) + start * size);
  ViewMoveTo(view, (double) start,  (double) *pt);

  for (i = start + 1; i <= end; i++) {

    pt = (float *)(((char *) array) + i * size);
	if ( *pt >= view->user_bottom && *pt <= view->user_top ){
	  ViewLineTo(view, (double) i, (double) *pt);
	}
  }
}

/***************************************************************************/

void ViewPlotClippedFloats ( View view, float *array, 
		       unsigned start, unsigned end, unsigned size ) {
  register unsigned i;
  register float	*pt;

  pt = (float *)(((char *) array) + start * size);
  ViewMoveTo(view, (double) start,  *pt);

  for (i = start + 1; i <= end; i += view_array_step ) {

    pt = (float *)(((char *) array) + i * size);
    if ( *pt > view->user_top ) ViewLineTo(view, (double) i, view->user_top );
    else if ( *pt < view->user_bottom ) ViewLineTo(view, (double) i, view->user_bottom );
    else ViewLineTo(view, (double) i, *pt);
	
  }
}

/***************************************************************************/

void ViewPlotAvailableFloats ( View view, float *array, 
							   unsigned start, unsigned end, 
							   unsigned size, float NA ) {

  register unsigned i;
  register float	*pt;
  int penon = NO;

  for (i = start; i <= end; i++) {

    pt = (float *)(((char *) array) + i * size);

    if ( *pt != NA && ( *pt >= view->user_bottom && *pt <= view->user_top ) ) {

      if ( penon == YES ) ViewLineTo( view, (double) i, (double) *pt);
      else ViewMoveTo( view, (double) i, (double) *pt );
      penon = YES;

    }

    else penon = NO;

  }
}



/***************************************************************************/

void ViewPlotFloatsDashed (View view, float *array, 
			   unsigned start, unsigned end, 
			   unsigned size, double on_size, double off_size ) {
					
  register unsigned i;
  register float *pt;

  Display dsp = view->display;

  double dx, dy, distance, desired, d;
  double dash_size = on_size * 10.0 * (double) dsp->symbol_radius;
  double break_size = off_size * 10.0 * (double) dsp->symbol_radius;
  float  x1, y1, x2, y2, x3, y3;

  pt = (float *)(((char *) array) + start * size);
  x1 = ViewUserToDisplayX( view, start );
  y1 = ViewUserToDisplayY( view, *pt );
  Point( dsp, x1, y1 );

  i = start + 1;
  pt = (float *)(((char *) array) + i * size);
  x2 = ViewUserToDisplayX( view, i );
  y2 = ViewUserToDisplayY( view, *pt );

  while ( i <= end ) {

    distance = 0.0;
    while ( i <= end && distance <= dash_size ) {

      dx = x2 - x1;
      dy = y2 - y1;
      d = sqrt( dx * dx + dy * dy );
      if ( distance + d <= dash_size ) {
	Line( dsp, x1, y1, x2, y2 );
	distance += d;
	x1 = x2;
	y1 = y2;
	i++;
	pt = (float *)(((char *) array) + i * size);
	x2 = ViewUserToDisplayX( view, i );
	y2 = ViewUserToDisplayY( view, *pt );
      }
      else {
	desired = dash_size - distance;
	x3 = x1 + dx * desired / d;
	y3 = y1 + dy * desired / d;
	Line( dsp, x1, y1, x3, y3 );
	x1 = x3;
	y1 = y3;
	break;
      }

    }

    distance = 0.0;
    while ( i <= end && distance <= break_size ) {

      dx = x2 - x1;
      dy = y2 - y1;
      d = sqrt( dx * dx + dy * dy );
      if ( distance + d <= break_size ) {
	distance += d;
	x1 = x2;
	y1 = y2;
	i++;
	pt = (float *)(((char *) array) + i * size);
	x2 = ViewUserToDisplayX( view, i );
	y2 = ViewUserToDisplayY( view, *pt );
      }
      else {
	desired = break_size - distance;
	x3 = x1 + dx * desired / d;
	y3 = y1 + dy * desired / d;
	x1 = x3;
	y1 = y3;
	break;
      }
    }
  }
}

/*****************************************************************************/

void ViewPlotInts ( View view, int *array, 
		      unsigned start, unsigned end, unsigned size ) {
					
  register unsigned i;
  register int		*pt;

  pt = (int *)(((char *) array) + start * size);
  ViewMoveTo(view, (double) start,  (double) *pt);

  for (i = start + 1; i <= end; i++) {

    pt = (int *)(((char *) array) + i * size);
    ViewLineTo(view, (double) i, (double) *pt);
	
  }
}

/***************************************************************************/

void ViewPlotAvailableInts ( View view, int *array, 
							 unsigned start, unsigned end, 
							 unsigned size, int NA ) {
  register unsigned i;
  register int  	*pt;
  int penon = NO;

  for (i = start; i <= end; i++) {

    pt = (int *)(((char *) array) + i * size);
    if ( *pt != NA ) {
      if ( penon == YES ) ViewLineTo( view, (double) i, (double) *pt);
      else ViewMoveTo( view, (double) i, (double) *pt );
      penon = YES;
    }
    else penon = NO;

  }
}

/***************************************************************************/

void ViewPointPlotAvailableInts ( View view, int *array, 
							 unsigned start, unsigned end, 
							 unsigned size, int NA ) {
  register unsigned i;
  register int  	*pt;

  for (i = start; i <= end; i++) {

    pt = (int *)(((char *) array) + i * size);
    if ( *pt != NA ) {
	  ViewPoint( view, (double) i, (double) *pt );
    }

  }
}

/*****************************************************************************/

void ViewPlotShorts ( View view, short *array, 
		      unsigned start, unsigned end, unsigned size ) {
					
  register unsigned i;
  register short	*pt;

  pt = (short *)(((char *) array) + start * size);
  ViewMoveTo(view, (double) start,  (double) *pt);

  for (i = start + 1; i <= end; i++) {

    pt = (short *)(((char *) array) + i * size);
    ViewLineTo(view, (double) i, (double) *pt);
	
  }
}

/***************************************************************************/

void ViewPlotChars ( View view, char *array, 
		     unsigned start, unsigned end, unsigned size ) {
					
  register unsigned i;
  register char	*pt;

  pt = (char *)(((char *) array) + start * size);
  ViewMoveTo(view, (double) start,  (double) *pt);

  for (i = start + 1; i <= end; i++) {

    pt = (char *)(((char *) array) + i * size);
    ViewLineTo(view, (double) i, (double) *pt);
	
  }
}

/***************************************************************************/

void ViewPlotAvailableChars ( View view, char *array, 
							 unsigned start, unsigned end, 
							 unsigned size, int NA ) {
  register unsigned i;
  register char  	*pt;
  int penon = NO;

  for (i = start; i <= end; i++) {

    pt = (char *)(((char *) array) + i * size);
    if ( *pt != NA ) {
      if ( penon == YES ) ViewLineTo( view, (double) i, (double) *pt);
      else ViewMoveTo( view, (double) i, (double) *pt );
      penon = YES;
    }
    else penon = NO;

  }
}

/*****************************************************************************/

/*
 * Autoscaling searches for the min and max within the given array.
 * It then expands the Y axis of the view to contain the range of values
 * that it finds.  The autoscale routines will not shrink the view smaller
 * than it already is. This allows multiple calls to a ViewAutoScale routine
 * to be used to scale to multiple arrays. Therefore, to start the process
 * one must call ViewAutoScaleInit() prior to calling ViewAutoScale???()
 * the first time.
 */

/*****************************************************************************/

void ViewAutoScaleInit( View view ) {
  /*
   * By setting the upper limit to the most negative possible value
   * and the lower limit to the most positive, the first value seen
   * in the array will be taken as the initial min and max.
   */
  ViewSetYLimits( view, HUGE, - HUGE );
}

/*****************************************************************************/

/*
 * Expand the Y range by a given percentage.
 * Usually used after AutoScaling to give an additional margin.
 */

void ViewAutoScaleExpand( View view, double fraction ) {

  double delta = fraction * ( view->user_top - view->user_bottom ) / 2.0;
	if ( delta == 0.0 ) delta = 0.0001;
  ViewSetYLimits( view, view->user_bottom - delta, view->user_top + delta );

}

/*****************************************************************************/

/*
 * Expand the Y range to a fixed interval around the midpoint of the measured values.
 * Usually used after AutoScaling to give an additional margin.
 */

void ViewAutoScaleSetInterval( View view, double interval ) {

  double mid = ( view->user_top + view->user_bottom ) / 2.0;
  ViewSetYLimits( view, mid - interval / 2, mid + interval / 2);

}

/*****************************************************************************/

void ViewAutoScaleShorts ( View view, short *array, 
			   unsigned start, unsigned end, unsigned size ) {
					
  register unsigned i;
  register short	*pt;
	
  short min, max;

//  if ( start < view->user_left ) start = view->user_left;
//  if ( end > view->user_right ) end = view->user_right;

  pt = (short *)(((char *) array) + start * size);
  if ( view->user_bottom > 32000 ) min = 32000; 
	else min = view->user_bottom;
  if ( view->user_top < -32000 ) max = -32000; 
	else max = view->user_top;
	
  for (i = start + 1; i <= end; i++) {

    pt = (short *)(((char *) array) + i * size);
    if ( *pt > max ) max = *pt;
    if ( *pt < min ) min = *pt;
	
  }
	
  ViewSetYLimits( view, min, max );
	
}

/***************************************************************************/

void ViewAutoScaleFloats ( View view, float *array, 
			   unsigned start, unsigned end, unsigned size ) {
					
  register unsigned i;
  register float	*pt;
	
  float min, max;

//  if ( start < view->user_left ) start = view->user_left;
//  if ( end > view->user_right ) end = view->user_right;

  pt = (float *)(((char *) array) + start * size);
  min = view->user_bottom;
  max = view->user_top;
	
  for (i = start + 1; i <= end; i++) {

    pt = (float *)(((char *) array) + i * size);
    if ( *pt > max ) max = *pt;
    if ( *pt < min ) min = *pt;
	
  }
	
  ViewSetYLimits( view, min, max );
	
}

/***************************************************************************/



void ViewAutoScaleAvailableFloats ( View view, float *array, 
				     unsigned start, unsigned end, unsigned size,
				     float NA ) {

  register unsigned i;
  register float   *pt;

  double min, max;

//  if ( start < view->user_left ) start = view->user_left;
//  if ( end > view->user_right ) end = view->user_right;

  pt = (float *)(((char *) array) + start * size);
  min = view->user_bottom;
  max = view->user_top;

  for (i = start + 1; i <= end; i++) {

    pt = (float *)(((char *) array) + i * size);

    if ( *pt != NA ) {
      if ( *pt >= max ) max = *pt;
      if ( *pt <= min ) min = *pt;

    }

  }

  ViewSetYLimits( view, min, max );

}

/***************************************************************************/

void ViewAutoScaleDoubles ( View view, double *array, 
			    unsigned start, unsigned end, unsigned size ) {
					
  register unsigned i;
  register double   *pt;
	
  double min, max;

//  if ( start < view->user_left ) start = view->user_left;
//  if ( end > view->user_right ) end = view->user_right;

  pt = (double *)(((char *) array) + start * size);
  min = view->user_bottom;
  max = view->user_top;
	
  for (i = start + 1; i <= end; i++) {

    pt = (double *)(((char *) array) + i * size);
    if ( *pt > max ) max = *pt;
    if ( *pt < min ) min = *pt;
	
  }
	
  ViewSetYLimits( view, min, max );
	
}


/***************************************************************************/

void ViewAutoScaleAvailableDoubles ( View view, double *array, 
				     unsigned start, unsigned end, unsigned size,
				     double NA ) {
					
  register unsigned i;
  register double   *pt;
	
  double min, max;

//  if ( start < view->user_left ) start = view->user_left;
//  if ( end > view->user_right ) end = view->user_right;

  pt = (double *)(((char *) array) + start * size);
  min = view->user_bottom;
  max = view->user_top;
	
  for (i = start + 1; i <= end; i++) {

    pt = (double *)(((char *) array) + i * size);

    if ( *pt != NA ) {
      if ( *pt > max ) max = *pt;
      if ( *pt < min ) min = *pt;
    }

  }
	
  ViewSetYLimits( view, min, max );
	
}

/*****************************************************************************/

/*
 * Expand the view to encompass most of the range, but 
 * discount outliers.
 */

void ViewAutoScaleMostDoubles ( View view,
					double *array, 
					unsigned start, 
					unsigned end, 
					unsigned size,
					double NA,
          double exclude ) {
					
	register unsigned i, j;
	double 	*list, *pt;
	int n, outlier_range, bytes;
	double min, max, margin;

	n = end - start + 1;
	list = (double *)malloc( ( bytes = n * sizeof( double ) ) );
	if ( !list ) {
		fprintf( stderr, "malloc failed for MedianAvailableDoubles()\n" );
		exit( - n );
	}

	pt = (double *)(((char *) array) + 0 * size);
	n = 1;
	list[0] = *pt;

	for (i = start + 1; i <= end; i++) {

		pt = (double *)(((char *) array) + i * size);
		if ( *pt != NA ) {
			for ( j = n; j > 0; j-- ) {
	      if ( *pt < list[j-1] ) list[j] = list[j-1];
	      else break;
			}
			list[j] = *pt;
			n++;
		}
	}
	outlier_range = (int) ((double) n * exclude );

  bytes = outlier_range;
	min = list[bytes];
  bytes = n - 1 - outlier_range;
	max = list[bytes];
	margin = exclude * ( max - min );
	min -= margin;
	max += margin;

	if ( view->user_bottom < min ) min = view->user_bottom;
	if ( view->user_top > max ) max =  view->user_top;

	ViewSetYLimits( view, min, max );
	
	free( list );


}

/***************************************************************************/

void ViewBoxPlotShorts (View view, short *array, 
			unsigned start, unsigned end, unsigned size ) {
					
  register unsigned i;
  register short	*pt;

  pt = (short *)(((char *) array) + start * size);
  ViewMoveTo(view, (double) start,  (double) *pt);
  ViewLineTo(view, (double) start + 0.5,  (double) *pt);

  for (i = start + 1; i <= end - 1; i++) {

    pt = (short *)(((char *) array) + i * size);
    ViewLineTo(view, (double) i - 0.5, (double) *pt);
    ViewLineTo(view, (double) i + 0.5, (double) *pt);
	
  }

  pt = (short *)(((char *) array) + end * size);
  ViewLineTo(view, (double) end - 0.5, (double) *pt);
  ViewLineTo(view, (double) end, (double) *pt);
	
}

/***************************************************************************/

void ViewBoxPlotFloats (View view, float *array, 
			unsigned start, unsigned end, unsigned size ) {
					
  register unsigned i;
  register float	*pt;

  pt = (float *)(((char *) array) + start * size);
  ViewMoveTo(view, (double) start,  (double) *pt);
  ViewLineTo(view, (double) start + 0.5,  (double) *pt);

  for (i = start + 1; i <= end - 1; i++) {

    pt = (float *)(((char *) array) + i * size);
    ViewLineTo(view, (double) i - 0.5, (double) *pt);
    ViewLineTo(view, (double) i + 0.5, (double) *pt);
	
  }

  pt = (float *)(((char *) array) + end * size);
  ViewLineTo(view, (double) end - 0.5, (double) *pt);
  ViewLineTo(view, (double) end, (double) *pt);
	
}

/***************************************************************************/

void ViewBoxPlotChars (View view, char *array, 
			unsigned start, unsigned end, unsigned size ) {
					
  register unsigned i;
  register char		*pt;

  pt = (char *)(((char *) array) + start * size);
  ViewMoveTo(view, (double) start,  0.0 );
  ViewLineTo(view, (double) start,  (double) *pt);
  ViewLineTo(view, (double) start + 0.5,  (double) *pt);
  ViewLineTo(view, (double) start + 0.5,  0.0 );

  for (i = start + 1; i <= end - 1; i++) {

    pt = (char *)(((char *) array) + i * size);
    ViewLineTo(view, (double) i - 0.5, (double) *pt);
    ViewLineTo(view, (double) i + 0.5, (double) *pt);
    ViewLineTo(view, (double) i + 0.5, 0.0 );
	
  }

  pt = (char *)(((char *) array) + end * size);
  ViewLineTo(view, (double) end - 0.5, (double) *pt);
  ViewLineTo(view, (double) end, (double) *pt);
  ViewLineTo(view, (double) end, 0.0 );
	
}

/***************************************************************************/

/*
	Plot two series of numbers, one vs. the other.  We have to do a lot of
	stuff to allow variable record lengths - xsize and ysize are the
	number of bytes between successive numbers - best computed with the
	sizeof() macro.
*/

void ViewXYPlotInts (View view, int *xarray, int *yarray, 
		     unsigned start, unsigned end, 
		     unsigned xsize, unsigned ysize) {
				  	
  register unsigned i;
  register int	*xpt, *ypt;

  xpt = (int *)(((char *) xarray) + start * xsize);
  ypt = (int *)(((char *) yarray) + start * ysize);
  ViewMoveTo(view,(double) *xpt, (double) *ypt);

  for (i = start; i <= end; i++) {

    xpt = (int *)(((char *) xarray) + i * xsize);
    ypt = (int *)(((char *) yarray) + i * ysize);
    ViewLineTo(view, (double) *xpt, (double) *ypt);

  }
}

/***************************************************************************/

void ViewXYPlotFloats (View view, float *xarray, float *yarray, 
		       unsigned start, unsigned end, 
		       unsigned xsize, unsigned ysize) {
				  	
  register unsigned i;
  register float	*xpt, *ypt;

  xpt = (float *)(((char *) xarray) + start * xsize);
  ypt = (float *)(((char *) yarray) + start * ysize);
  ViewMoveTo(view,(double) *xpt, (double) *ypt);

  for (i = start; i <= end; i++) {

    xpt = (float *)(((char *) xarray) + i * xsize);
    ypt = (float *)(((char *) yarray) + i * ysize);
    ViewLineTo(view, (double) *xpt, (double) *ypt);

  }
}

/***************************************************************************/

void ViewXYPlotDoubles (View view, double *xarray, double *yarray, 
			unsigned start, unsigned end, unsigned step,
			unsigned xsize, unsigned ysize) {
				   	
  register unsigned i;
  register double	*xpt, *ypt;

  xpt = (double *)(((char *) xarray) + start * xsize);
  ypt = (double *)(((char *) yarray) + start * ysize);
  ViewMoveTo(view, *xpt, *ypt);

  for (i = start; i <= end; i++) {

    xpt = (double *)(((char *) xarray) + i * xsize);
    ypt = (double *)(((char *) yarray) + i * ysize);
    ViewLineTo(view, *xpt, *ypt);

  }
}

/***************************************************************************/

void ViewXYPlotAvailableChars (View view, char *xarray, char *yarray, 
				unsigned start, unsigned end, 
				unsigned xsize, unsigned ysize, 
				float na)
{
	
  register unsigned i;
  register float	*xpt1, *ypt1, *xpt2, *ypt2;

  i = start;
  while (i < end) {

    xpt1 = (float *)(((char *) xarray) + i * xsize);
    ypt1 = (float *)(((char *) yarray) + i * ysize);
    xpt2 = (float *)(((char *) xarray) + (++i) * xsize);
    ypt2 = (float *)(((char *) yarray) + i * ysize);
    if (*xpt1 != na && *ypt1 != na && *xpt2 != na && *ypt2 != na)
      ViewLine(view, 
	       (double) *xpt1, (double) *ypt1,
	       (double) *xpt2, (double) *ypt2 );

  }
}
/***************************************************************************/

void ViewXYPlotAvailableFloats (View view, float *xarray, float *yarray, 
				unsigned start, unsigned end, 
				unsigned xsize, unsigned ysize, 
				float na)
{
	
  register unsigned i;
  register float	*xpt1, *ypt1, *xpt2, *ypt2;

  i = start;
  while (i < end) {

    xpt1 = (float *)(((char *) xarray) + i * xsize);
    ypt1 = (float *)(((char *) yarray) + i * ysize);
    xpt2 = (float *)(((char *) xarray) + (++i) * xsize);
    ypt2 = (float *)(((char *) yarray) + i * ysize);
    if (*xpt1 != na && *ypt1 != na && *xpt2 != na && *ypt2 != na)
      ViewLine(view, 
	       (double) *xpt1, (double) *ypt1,
	       (double) *xpt2, (double) *ypt2 );

  }
}

/***************************************************************************/

void ViewXYPlotAvailableDoubles (View view, double *xarray, double *yarray, 
				 unsigned start, unsigned end, unsigned step,
				 unsigned xsize, unsigned ysize, 
				 double na)
{
	
  register          unsigned i;
  register double   *xpt1, *ypt1, *xpt2, *ypt2;

  i = start;
  while (i <= (end - step) ) {

    xpt1 = (double *)(((char *) xarray) + i * xsize);
    ypt1 = (double *)(((char *) yarray) + i * ysize);
	i += step;
    xpt2 = (double *)(((char *) xarray) + i * xsize);
    ypt2 = (double *)(((char *) yarray) + i * ysize);
    if (*xpt1 != na && *ypt1 != na && *xpt2 != na && *ypt2 != na) {
      ViewLine(view, 
	       (double) *xpt1, (double) *ypt1,
	       (double) *xpt2, (double) *ypt2 );
	}
  }
}
void ViewXYPlotClippedDoubles (View view, double *xarray, double *yarray, 
				 unsigned start, unsigned end, unsigned step,
				 unsigned xsize, unsigned ysize, 
				 double na)
{
	
  register          unsigned i;
  register double   *xpt1, *ypt1, *xpt2, *ypt2;

  i = start;
  while (i <= (end - step) ) {

    xpt1 = (double *)(((char *) xarray) + i * xsize);
    ypt1 = (double *)(((char *) yarray) + i * ysize);
	i += step;
    xpt2 = (double *)(((char *) xarray) + i * xsize);
    ypt2 = (double *)(((char *) yarray) + i * ysize);
    if (*xpt1 != na && *xpt1 >= view->user_left && *xpt1 <= view->user_right &&
		*xpt2 != na && *xpt2 >= view->user_left && *xpt2 <= view->user_right &&
		*ypt1 != na && *ypt1 >= view->user_bottom && *ypt1 <= view->user_top &&
		*ypt2 != na && *ypt2 >= view->user_bottom && *ypt2 <= view->user_top ) {
      ViewLine(view, 
	       (double) *xpt1, (double) *ypt1,
	       (double) *xpt2, (double) *ypt2 );
	}
  }
}

/***************************************************************************/
/*                              Scatter Plots                              */
/***************************************************************************/

void ViewScatterPlotInts (	View view, int symbol,
				int *xarray, int *yarray, 
				unsigned start, unsigned end, 
				unsigned xsize, unsigned ysize)
{

  register unsigned i;
  register int	*xpt, *ypt;

  for (i = start; i <= end; i++) {

    xpt = (int *)(((char *) xarray) + i * xsize);
    ypt = (int *)(((char *) yarray) + i * ysize);
    ViewSymbol(view, (double) *xpt, (double) *ypt, symbol );

  }
}

/***************************************************************************/

void ViewScatterPlotFloats (View view, int symbol,
			    float *xarray, float *yarray, 
			    unsigned start, unsigned end, 
			    unsigned xsize, unsigned ysize)
{

  register unsigned i;
  register float	*xpt, *ypt;

  for (i = start; i <= end; i++) {

    xpt = (float *)(((char *) xarray) + i * xsize);
    ypt = (float *)(((char *) yarray) + i * ysize);
    ViewSymbol(view, (double) *xpt, (double) *ypt, symbol);

  }
}

/***************************************************************************/

void ViewScatterPlotAvailableFloats (View view, int symbol,
				     float *xarray, float *yarray, 
				     unsigned start, unsigned end, 
				     unsigned xsize, unsigned ysize,
				     float NA )
{

  register unsigned i;
  register float	*xpt, *ypt;

  for (i = start; i <= end; i++) {

    xpt = (float *)(((char *) xarray) + i * xsize);
    ypt = (float *)(((char *) yarray) + i * ysize);
    if ( *xpt != NA && *ypt != NA ) {
      ViewSymbol(view, (double) *xpt, (double) *ypt, symbol);
    }

  }
}

/***************************************************************************/

void ViewScatterPlotDoubles (View view, int symbol,
			     double *xarray, double *yarray, 
			     unsigned start, unsigned end, 
			     unsigned xsize, unsigned ysize)
{
  register unsigned i;
  register double	*xpt, *ypt;

  for (i = start; i <= end; i++) {

    xpt = (double *)(((char *) xarray) + i * xsize);
    ypt = (double *)(((char *) yarray) + i * ysize);
    ViewSymbol(view, *xpt, *ypt, symbol);

  }
}

/***************************************************************************/

void ViewScatterPlotAvailableDoubles (View view, int symbol,
				     double *xarray, double *yarray, 
				     unsigned start, unsigned end, unsigned step,
				     unsigned xsize, unsigned ysize,
				     double NA )
{

  register unsigned i;
  register double	*xpt, *ypt;

  for (i = start; i <= end; i += step ) {

    xpt = (double *)(((char *) xarray) + i * xsize);
    ypt = (double *)(((char *) yarray) + i * ysize);
    if ( *xpt != NA && *ypt != NA ) {
      ViewSymbol(view, (double) *xpt, (double) *ypt, symbol);
    }

  }
}

