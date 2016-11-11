/***************************************************************************/
/*                                                                         */
/*                             FunctionPlots.c                             */
/*                                                                         */
/***************************************************************************/

/*    (c) Copyright 1994, 1995, 2014 - PsyPhy Consulting. All Rights Reserved.    */

/*	Plot functions within a view. */

#include <stdio.h>
#include <math.h>
#include <windows.h>

#include "useful.h"
#include <gl/gl.h>
#include "glut.h"

#include "Displays.h"
#include "Views.h"

/***************************************************************************/

void ViewPlotPolynomial (View view, double p[], int n ) {

  double x, y, xpwr;
  double y_min, y_max, step;

  int i;

  if ( view->user_top > view->user_bottom ) {
    y_min = view->user_bottom;
    y_max = view->user_top;
  }
  else {
    y_max = view->user_bottom;
    y_min = view->user_top;
  }
    
  step = ( view->user_right - view->user_left ) /
    ( view->display_right - view->display_left );

  for ( x = view->user_left; x <= view->user_right; x += step ) {

    y = p[0];
    xpwr = 1;
    for ( i = 1; i <= n; i++ ) {
      xpwr *= x;
      y += xpwr * p[i];
    }
    if ( y > y_min && y < y_max ) {
      ViewMoveTo( view, x, y );
      break;
    }
    
  }

  for ( x = view->user_left; x <= view->user_right; x += step ) {

    y = p[0];
    xpwr = 1;
    for ( i = 1; i <= n; i++ ) {
      xpwr *= x;
      y += xpwr * p[i];
    }
    if ( y > y_min && y < y_max ) ViewLineTo( view, x, y );
    
  }
}

/***************************************************************************/

void ViewPlotLine (View view, double slope, double intercept) {

  double	x1, x2, y1, y2;
	
  if ( view->user_top > view->user_bottom ) {

    x1 = view->user_left;
    y1 = slope * x1 + intercept;
    if (y1 > view->user_top) {
      y1 = view->user_top;
      x1 = (y1 - intercept) / slope;
    }
    if (y1 < view->user_bottom) {
      y1 = view->user_bottom;
      x1 = (y1 - intercept) / slope;
    }
		
    x2 = view->user_right;
    y2 = slope * x2 + intercept;
    if (y2 > view->user_top) {
      y2 = view->user_top;
      x2 = (y2 - intercept) / slope;
    }
    if (y2 < view->user_bottom) {
      y2 = view->user_bottom;
      x2 = (y2 - intercept) / slope;
    }
		
  }
  else {

    x1 = view->user_left;
    y1 = slope * x1 + intercept;
    if (y1 < view->user_top) {
      y1 = view->user_top;
      x1 = (y1 - intercept) / slope;
    }
    if (y1 > view->user_bottom) {
      y1 = view->user_bottom;
      x1 = (y1 - intercept) / slope;
    }
		
    x2 = view->user_right;
    y2 = slope * x2 + intercept;
    if (y2 < view->user_top) {
      y2 = view->user_top;
      x2 = (y2 - intercept) / slope;
    }
    if (y2 > view->user_bottom) {
      y2 = view->user_bottom;
      x2 = (y2 - intercept) / slope;
    }
  }

  ViewLine(view, x1, y1, x2, y2);
	
}

/***************************************************************************/

void ViewPlotLineYX( View view, double slope, double intercept) {

  double	x1, x2, y1, y2;

  if ( view->user_top > view->user_bottom ) {

    y1 = view->user_bottom;
    x1 = slope * y1 + intercept;
    if (x1 > view->user_right) {
      x1 = view->user_right;
      y1 = (x1 - intercept) / slope;
    }
    if (x1 < view->user_left) {
      x1 = view->user_left;
      y1 = (x1 - intercept) / slope;
    }
		
    y2 = view->user_top;
    x2 = slope * y2 + intercept;
    if (x2 > view->user_right) {
      x2 = view->user_right;
      y2 = (x2 - intercept) / slope;
    }
    if (x2 < view->user_left) {
      x2 = view->user_left;
      y2 = (x2 - intercept) / slope;
    }

  }
  else {

    y1 = view->user_top;
    x1 = slope * y1 + intercept;
    if (x1 > view->user_right) {
      x1 = view->user_right;
      y1 = (x1 - intercept) / slope;
    }
    if (x1 < view->user_left) {
      x1 = view->user_left;
      y1 = (x1 - intercept) / slope;
    }
		
    y2 = view->user_bottom;
    x2 = slope * y2 + intercept;
    if (x2 > view->user_right) {
      x2 = view->user_right;
      y2 = (x2 - intercept) / slope;
    }
    if (x2 < view->user_left) {
      x2 = view->user_left;
      y2 = (x2 - intercept) / slope;
    }

  }
		
  ViewLine(view, x1, y1, x2, y2);
	
}

/***************************************************************************/

/* This cannot be bigger that PSGL_MAX_GLVERTICES / 2 - 2  */
#define ELLIPSE_SIDES 240

void ViewPlotEllipse (View view, double matrix[2][2], double scale, double center[2] ) {

  double angle_step = 2.0 * PI / (double) ELLIPSE_SIDES;
  double	x, y, x0, y0, angle, delta[2];
  int i;
	
  ViewStartPolygon( view );
		
  delta[X] = scale * cos( 0.0 );
  delta[Y] = scale * sin( 0.0 );
  x0 = center[X] + matrix[X][X] * delta[X] + matrix[X][Y] * delta[Y];	
  y0 = center[Y] + matrix[Y][X] * delta[X] + matrix[Y][Y] * delta[Y];	
  ViewAddVertex( view, x0, y0 );

  for ( angle = angle_step, i = 1; i <= ELLIPSE_SIDES; 
	angle += angle_step, i++ ) {
	
    delta[X] = scale * cos( angle );
    delta[Y] = scale * sin( angle );
    x = center[X] + matrix[X][X] * delta[X] + matrix[X][Y] * delta[Y];	
    y = center[Y] + matrix[Y][X] * delta[X] + matrix[Y][Y] * delta[Y];	
    ViewAddVertex( view, x, y );
		
  }

  ViewAddVertex( view, x0, y0 );
  ViewOutlinePolygon( view );

}

void ViewPlotFilledEllipse (View view, double matrix[2][2], double scale, double center[2] ) {

  double angle_step = 2.0 * PI / (double) ELLIPSE_SIDES;
  double	x, y, x0, y0, angle, delta[2];
  int i;
	
  ViewStartPolygon( view );
		
  delta[X] = scale * cos( 0.0 );
  delta[Y] = scale * sin( 0.0 );
  x0 = center[X] + matrix[X][X] * delta[X] + matrix[X][Y] * delta[Y];	
  y0 = center[Y] + matrix[Y][X] * delta[X] + matrix[Y][Y] * delta[Y];	
  ViewAddVertex( view, x0, y0 );

  for ( angle = angle_step, i = 1; i <= ELLIPSE_SIDES; 
	angle += angle_step, i++ ) {
	
    delta[X] = scale * cos( angle );
    delta[Y] = scale * sin( angle );
    x = center[X] + matrix[X][X] * delta[X] + matrix[X][Y] * delta[Y];	
    y = center[Y] + matrix[Y][X] * delta[X] + matrix[Y][Y] * delta[Y];	
    ViewAddVertex( view, x, y );
		
  }

  ViewAddVertex( view, x0, y0 );
  ViewFillPolygon( view );

}
#if 0
extern void invert_matrix_2d( const double matrix[2][2], double inverse[2][2] );


void ViewPlotSquaredEllipse (View view, double matrix[2][2], 
			     double scale, double center[2] ) {

  double angle_step = 2.0 * PI / ELLIPSE_SIDES;
  double	x, y, x0, y0, angle, denominator, inverse[2][2];
  int i;
	
  invert_matrix_2d( matrix, inverse);
  ViewStartPolygon( view );
		
  x0 = cos( 0.0 );
  y0 = sin( 0.0 );

  denominator = sqrt( inverse[X][X] * x0 * x0 +
		      inverse[Y][Y] * y0 * y0 +
		      2.0 * inverse[X][Y] * x0 * y0 );

  x0 *= scale / denominator;
  y0 *= scale / denominator;
  ViewAddVertex( view, x0 + center[X], y0  + center[Y] );

  for ( angle = angle_step, i = 1; 
	i <= ELLIPSE_SIDES; 
	angle += angle_step, i++ ) {
	
    x = cos( angle );
    y = sin( angle );

    denominator = sqrt( inverse[X][X] * x * x +
			inverse[Y][Y] * y * y +
			2.0 * inverse[X][Y] * x * y );

    x *= scale / denominator;
    y *= scale / denominator;
    ViewAddVertex( view, x + center[X], y  + center[Y] );
		
  }

  ViewAddVertex( view, x0 + center[X], y0  + center[Y] );
  ViewOutlinePolygon( view );

}


void ViewPlotSpectrumEllipse (View view, double matrix[2][2], double scale, double center[2],
                              double (*fn)( void *ptr, double angle  ), void *ptr ) {

  double angle_step = 2.0 * PI / ELLIPSE_SIDES;
  double l;
  double	x, y, x0, y0, angle, denominator, inverse[2][2];
  int i;
	
  invert_matrix_2d( matrix, inverse);
		 
  x0 = cos( 0.0 );
  y0 = sin( 0.0 );

  denominator = sqrt( inverse[X][X] * x0 * x0 +
		      inverse[Y][Y] * y0 * y0 +
		      2.0 * inverse[X][Y] * x0 * y0 );

  x0 *= scale / denominator;
  y0 *= scale / denominator;

  for ( angle = angle_step, i = 1; 
	i <= ELLIPSE_SIDES; 
	angle += angle_step, i++ ) {
	
    x = cos( angle );
    y = sin( angle );

    denominator = sqrt( inverse[X][X] * x * x +
			inverse[Y][Y] * y * y +
			2.0 * inverse[X][Y] * x * y );

    x *= scale / denominator;
    y *= scale / denominator;

    l = (*fn)( ptr, angle );
    ViewSetSpectrumColor( view, l );


    ViewStartPolygon( view );
    ViewAddVertex( view, center[X], center[Y] );
    ViewAddVertex( view, x + center[X], y  + center[Y] );
    ViewAddVertex( view, x0 + center[X], y0  + center[Y] );
    ViewAddVertex( view, center[X], center[Y] );
    ViewFillPolygon( view );

    x0 = x;
    y0 = y;
		
  }


}
void ViewPlotSpectrumCircle (View view, double x, double y, double radius, 
                             double (*fn)( void *ptr, double angle ), void *ptr ) {

  double center[2];
  static double ellipse[2][2] = { 1.0, 0.0, 0.0, 1.0 };

  center[X] = x;
  center[Y] = y;

  ViewPlotSpectrumEllipse( view, ellipse, radius, center, fn, ptr );

}
#endif 


#if 0
double angle_step = .025;

void ViewPlotEllipse (View view, double matrix[2][2], double scale, double center[2] ) {

  double	x, y, angle, delta[2];
	
  x = center[X] + matrix[X][X] * scale;
  y = center[Y] + matrix[Y][X] * scale;
  ViewMoveTo( view, x, y );
		
  for ( angle = 0; angle < 2 * PI; angle += angle_step ) {
	
    delta[X] = scale * cos( angle );
    delta[Y] = scale * sin( angle );
    x = center[X] + matrix[X][X] * delta[X] + matrix[X][Y] * delta[Y];	
    y = center[Y] + matrix[Y][X] * delta[X] + matrix[Y][Y] * delta[Y];	
    ViewLineTo( view, x, y );
		
  }
}

void ViewPlotSquaredEllipse (View view, double matrix[2][2], double scale, double center[2] ) {

  double angle_step = 2.0 * PI / 256;
  double	x, y, angle, distance, delta[2], ca, sa;
	
  ca = cos( 0.0 );
  sa = sin( 0.0 );
  delta[X] = matrix[X][X] * ca + matrix[X][Y] * sa;
  delta[Y] = matrix[Y][X] * ca + matrix[Y][Y] * sa;
  x = center[X] + scale * sqrt( delta[X] ) * ca;
  y = center[Y] + scale * sqrt( delta[Y] ) * sa;
  ViewMoveTo( view, x, y );
		
  for ( angle = 0.0; angle <= 2 * PI + angle_step ; angle += angle_step ) {
	
    ca = cos( angle );
    sa = sin( angle );
    delta[X] = matrix[X][X] * ca + matrix[X][Y] * sa;
    delta[Y] = matrix[Y][X] * ca + matrix[Y][Y] * sa;
    x = center[X] + scale * sqrt( delta[X] ) * ca;
    y = center[Y] + scale * sqrt( delta[Y] ) * sa;
    ViewLineTo( view, x, y );
		
  }
}
#endif
