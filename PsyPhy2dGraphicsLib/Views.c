/****************************************************************************/
/*                                                                          */
/*                                 Views.c                                  */
/*                                                                          */
/****************************************************************************/

/*    (c) Copyright 1994, 1995 - PsyPhy Consulting. All Rights Reserved.    */

// Defines that increase the amount of info in the memory leak report.
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "Graphics.h"
#include "Displays.h"
#include "Views.h"


View _view_destroy_list = NULL;

/****************************************************************************/

View CreateView (Display display) {
	
	View	view;
	
	view = (View)malloc(sizeof( struct _view ));
	
	if (!view) {
		fprintf(stderr, "Error allocating space for View.\n");
		return(0);
	}
	
	view->display = display;
	view->next = _view_destroy_list;
	_view_destroy_list = view;
	
	ViewInit(view);
	
	return(view);
}

void DestroyViews ( void ) {
	View view = _view_destroy_list;
	View hold;
	while ( view ) {
		hold = view;
		view = view->next;
		free( hold );
	}
}

/****************************************************************************/

void	ViewInit (View view) {
	
	view->display_left = view->display->left;
	view->display_right = view->display->right;
	view->display_top = view->display->top;
	view->display_bottom = view->display->bottom;
	
	view->user_left   = -1.0;
	view->user_right  =  1.0;
	view->user_top    =  1.0;
	view->user_bottom = -1.0;
	
	view->user_min_depth = 0.0;
	view->user_max_depth = 1.0;
	
	ViewComputeFactors(view);
	
}

/****************************************************************************/

void	ViewComputeFactors (View view) {
	
	view->x_factor = ((double)(view->display_right - view->display_left)) /
		(view->user_right - view->user_left);
	
	view->y_factor = ((double)(view->display_top - view->display_bottom)) /
		(view->user_top - view->user_bottom);
	
	view->x_sense = view->display_right > view->display_left ? 1 : -1;
	view->y_sense = view->display_bottom > view->display_top ? 1 : -1;
	
}

/****************************************************************************/

void	ViewDescribe (View view) {
	
	fprintf(stderr, "View: %lx    Display: %lx \n",
		(long) view, (long) view->display);
	fprintf(stderr, "Display Left: %5.2f    User Left: %7.3f\n",
		view->display_left, view->user_left);
	fprintf(stderr, "       Right: %5.2f        Right: %7.3f\n",
		view->display_right, view->user_right);
	fprintf(stderr, "         Top: %5.2f          Top: %7.3f\n",
		view->display_top, view->user_top);
	fprintf(stderr, "      Bottom: %5.2f        Bottom: %7.3f\n",
		view->display_bottom, view->user_bottom);
	
	fprintf(stderr, "\nX Factor: %7.3f   Y Factor: %7.3f\n",
		view->x_factor, view->y_factor);
	fprintf(stderr, "X Sense: %d   Y Sense: %d\n", 
		view->x_sense, view->y_sense);
	
}

/***************************************************************************/

View ViewDisplayPointInside( View view, int x, int y ) {
	
	if ( view->x_sense * x > view->x_sense * view->display_left &&
		view->x_sense * x < view->x_sense * view->display_right &&
		view->y_sense * y > view->y_sense * view->display_top &&
		view->y_sense * y < view->y_sense * view->display_bottom ) {
		return( view );
	}
	else return ( NULL );
	
}

/***************************************************************************/

void ViewSetDisplayEdges (View view, int left, int bottom, int right, int top)

{
	
	view->display_left = left;
	view->display_right = right;
	view->display_top = top;
	view->display_bottom = bottom;
	
	ViewComputeFactors(view);
	
}

/***************************************************************************/

void ViewShiftRelative ( View view, double x, double y ) {
	
	double right = x * ( view->display->left - view->display->right );
	double up    = y * ( view->display->top - view->display->bottom );
	
	view->display_left += right;
	view->display_right += right;
	view->display_top += up;
	view->display_bottom += up;
	ViewComputeFactors(view);
	
}

/***************************************************************************/

void ViewSetDisplayEdgesRelative (View view, 
								  double left, double bottom, 
								  double right, double top) {
	
	double	border;
	
	border = view->display->right > view->display->left ? 1 : -1; 
	view->display_left = view->display->left + border +
		left * (view->display->right - view->display->left);
	view->display_right = view->display->left - border +
		right * (view->display->right - view->display->left);
	
	border = view->display->bottom > view->display->top ? 1 : -1;
	view->display_top = view->display->top + border +
		(1.0 - top) * (view->display->bottom - view->display->top);
	view->display_bottom = view->display->top - border +
		(1.0 - bottom) * (view->display->bottom - view->display->top);
	
	ViewComputeFactors(view);
	
}

/***************************************************************************/

void ViewSetEdges (View view, double left, double bottom, 
				   double right, double top)	{
	
	view->user_left  = left;
	view->user_right = right;
	view->user_top   = top;
	view->user_bottom= bottom;
	
	ViewComputeFactors(view);
	
}

/***************************************************************************/

void ViewSetXLimits (View view, double left, double right ) {
	
	view->user_left  = left;
	view->user_right = right;
	ViewComputeFactors(view);
	
}

void ViewSetYLimits (View view, double bottom, double top ) {
	
	view->user_top   = top;
	view->user_bottom= bottom;
	ViewComputeFactors(view);
	
}

void ViewSetDepthLimits( View view, double min, double max ) {
	
	view->user_min_depth = min;
	view->user_max_depth = max;
	
}

/***************************************************************************/

double ViewYRange( View view ) {
	return( fabs( view->user_top - view->user_bottom ) ) ;
}

void ViewSetYRange( View view, double range ) {
	
	double mid = ( view->user_top + view->user_bottom ) / 2.0;
	
	if ( view->user_top > view->user_bottom ) {
		ViewSetYLimits( view, mid - range / 2.0, mid + range / 2.0 );
	}
	else {
		ViewSetYLimits( view, mid + range / 2.0, mid - range / 2.0 );
	}
}

void ViewAdjustYRange( View view, double factor ) {
	
	ViewSetYRange( view, factor * ViewYRange( view ) );
	
}


void ViewCenterZeroY( View view ) {
	
	double range = max( fabs( view->user_top ), fabs( view->user_bottom) );
	
	ViewSetYLimits( view, - range, range );
	
}



/***************************************************************************/

void ViewMakeSquare (View view) {
	
	register	center, width, height, new_width, new_height;
	
	if (fabs(view->x_factor) > fabs(view->y_factor)) {
		
		center = (view->display_left + view->display_right) / 2;
		width = abs(view->display_right - view->display_left);
		new_width = fabs(view->y_factor / view->x_factor * (double) width); 
		
		if (view->display->right > view->display->left)
			ViewSetDisplayEdges(view, center - new_width / 2, view->display_bottom,
			center + new_width / 2, view->display_top);
		else
			ViewSetDisplayEdges(view, center + new_width / 2, view->display_bottom,
			center - new_width / 2, view->display_top);
		
	}
	
	else {
		
		center = (view->display_bottom + view->display_top) / 2;
		height = abs(view->display_bottom - view->display_top);
		new_height = fabs(view->x_factor / view->y_factor * (double) height);
		
		if (view->display->bottom > view->display->top)
			ViewSetDisplayEdges(view, 
			view->display_left, center + new_height / 2,
			view->display_right, center - new_height / 2);
		else
			ViewSetDisplayEdges(view, 
			view->display_left, center - new_height / 2,
			view->display_right, center + new_height / 2);
		
	}
}



/***************************************************************************/

void ViewPoint (View view, double x, double y) {
	
	Point(view->display, UserToDisplayX(view, x), UserToDisplayY(view, y));
	
}

/***************************************************************************/

void ViewSymbol( View view, double x, double y, int symbol ) {
	
	DisplaySymbol( view->display, 
		UserToDisplayX(view, x), UserToDisplayY(view, y), symbol );
	
}

/***************************************************************************/

void ViewMoveTo (View view, double x, double y)	{
	
	Moveto(view->display, UserToDisplayX(view, x), UserToDisplayY(view, y));
	
}

/***************************************************************************/

void ViewLineTo (View view, double x, double y)	{
	
	Lineto(view->display, UserToDisplayX(view, x), UserToDisplayY(view, y));
	
}

/***************************************************************************/

void ViewLine (View view, double from_x, double from_y, 
			   double to_x, double to_y)	{
	
	Line(view->display,
		UserToDisplayX(view, from_x), UserToDisplayY(view, from_y),
		UserToDisplayX(view, to_x),   UserToDisplayY(view, to_y));
	
}

/***************************************************************************/

void ViewArrow (View view, double from_x, double from_y, 
				double to_x, double to_y)	{
	
	DisplayArrow(view->display,
		UserToDisplayX(view, from_x), UserToDisplayY(view, from_y),
		UserToDisplayX(view, to_x),   UserToDisplayY(view, to_y));
	
}

/***************************************************************************/

void ViewText (View view, char *string, double x, double y, double dir)	{
	
	Text(view->display, string,
		UserToDisplayX(view, x), UserToDisplayY(view, y), dir);
	
}

/***************************************************************************/

void ViewTitle (View view, char *string, int x, int y, double dir) {
	
/*
A value less than 0 for x or y implies a keyword.
    */
	switch (x) {
		
	case (INSIDE_LEFT):
		x = view->display_left + 1;
		break;
		
	case (OUTSIDE_LEFT):
		x = view->display_left - 
			view->x_sense * TextWidth(view->display, string) - 1;
		break;
		
	case (INSIDE_RIGHT):
		x = view->display_right - 
			view->x_sense * TextWidth(view->display, string) - 1;
		break;
		
	case (OUTSIDE_RIGHT):
		x = view->display_right + 1;
		break;
		
	case (CENTER):
		x = ( view->display_left + view->display_right 
			- view->x_sense * TextWidth(view->display, string)) / 2;
		break;
		
	default:
		x = view->display_top + view->x_sense * x;
		break;
		
	}
	
	
	switch (y) {
		
	case (INSIDE_BOTTOM):
		y = view->display_bottom - view->y_sense * 1;
		break;
		
	case (OUTSIDE_BOTTOM):
		y = view->display_bottom + 
			view->y_sense * TextHeight(view->display, string) + 1;
		break;
		
	case (INSIDE_TOP):
		y = view->display_top +
			view->y_sense * TextHeight(view->display, string) + 1;
		break;
		
	case (OUTSIDE_TOP):
		y = view->display_top -
			view->y_sense * ( 0.3 * TextHeight(view->display, string) + 1 );
		break;
		
	case (CENTER):
		y = ( view->display_bottom + view->display_top 
			+ view->y_sense 
			* TextHeight(view->display, string)) / 2;
		break;
		
	default:
		y = view->display_top + view->y_sense * y;
		break;
		
	}
	
	DisplayTitle( view->display, string, x, y, dir);
	
}

/***************************************************************************/

void ViewLabel (View view, char *string, 
				double x, double y, double dir, 
				int x_justify, int y_justify ) {
	
	DisplayLabel(view->display, string, 
	       UserToDisplayX(view, x), 
		   UserToDisplayY(view, y), 
		   dir, x_justify, y_justify);
	
}

/***************************************************************************/

void ViewRectangle (View view, double x1, double y1, double x2, double y2) {
	
	Rectangle(view->display,
		UserToDisplayX(view, x1), UserToDisplayY(view, y1),
		UserToDisplayX(view, x2), UserToDisplayY(view, y2));
	
}
/***************************************************************************/

void ViewFilledRectangle (View view, double x1, double y1, double x2, double y2) {
	
	FilledRectangle(view->display,
		UserToDisplayX(view, x1), UserToDisplayY(view, y1),
		UserToDisplayX(view, x2), UserToDisplayY(view, y2));
	
}

/***************************************************************************/

void ViewFilledCircle (View view, double x, double y, double radius ) {
	
	FilledCircle(view->display,
	       UserToDisplayX(view, x), UserToDisplayY(view, y),
		   abs( UserToDisplayX(view, radius ) - UserToDisplayX(view, 0.0)) );
	
}

/***************************************************************************/

void ViewCircle (View view, double x, double y, double radius ) {
	
	Circle(view->display,
		UserToDisplayX(view, x), 
		UserToDisplayY(view, y),
		abs( UserToDisplayX(view, radius ) - UserToDisplayX(view, 0.0)) );
	
}

/***************************************************************************/

void ViewStartPolygon ( View view ) {
	
	StartPolygon( view->display );
	
}

void ViewAddVertex ( View view, double x, double y ) {
	
	AddVertex( view->display, UserToDisplayX( view, x ),
		UserToDisplayY( view, y ) );
	
}

void ViewOutlinePolygon ( View view ) {
	
	OutlinePolygon( view->display );
	
}

void ViewFillPolygon ( View view ) {
	
	FillPolygon( view->display );
	
}


/***************************************************************************/

void ViewErase (View view) {
	
	EraseRectangle(view->display, view->display_left, view->display_top,
		view->display_right, view->display_bottom);
	
}

/***************************************************************************/

void ViewBox (View view) {
	
	Line(view->display, view->display_left, view->display_top,
		view->display_left, view->display_bottom);
	Line(view->display, view->display_left, view->display_bottom,
		view->display_right, view->display_bottom);
	Line(view->display, view->display_right, view->display_bottom,
		view->display_right, view->display_top);
	Line(view->display, view->display_right, view->display_top,
		view->display_left, view->display_top);
	
}

/***************************************************************************/

void ViewLineStyle (View view, int style) {
	
	LineStyle(view->display, style);
	
}

/***************************************************************************/

void ViewLinePattern (View view, int pattern) {
	
	LinePattern(view->display, pattern);
	
}

/***************************************************************************/

void ViewAlu (View view, int alu) {
	
	Alu(view->display, alu);
	
}


/***************************************************************************/

void ViewColor (View view, int color) {
	
	Color(view->display, color);
	
}

void ViewSelectColor (View view, int color) {
	
	Color( view->display, 
		color_select_table[ color % ( sizeof( color_select_table ) / sizeof( *color_select_table ) ) ]
		);
	
}

/***************************************************************************/

/* These are used for pseudo-color plots. */

void ViewSetSpectrumColor( View view, double value ) {
	
	double sigma_red = 0.35;
	double sigma_green = 0.35;
	double sigma_blue = 0.25;
	
	double center_red = 0.55;
	double center_green = 0.0;
	double center_blue = -0.45;
	
	double r, g, b;
	
	double relative_value = ( value - view->user_min_depth ) / ( view->user_max_depth - view->user_min_depth );
	
	r = exp( - (relative_value - center_red) * (relative_value - center_red) / sigma_red );
	g = exp( - (relative_value - center_green) * (relative_value - center_green) / sigma_green );
	b = exp( - (relative_value - center_blue) * (relative_value - center_blue) / sigma_blue );
	ColorRGB( view->display, r, g, b );
	
}

void ViewSetPseudoColorPixels( View view, int x_pixels, int y_pixels ) {
	view->pseudocolor_x_radius = ViewWidth( view ) / (double) ( x_pixels - 1 ) / 2.0;
	view->pseudocolor_y_radius = ViewHeight( view ) / (double) ( y_pixels - 1 ) / 2.0;
}


void ViewPlotPseudoColor( View view, double x, double y, double value ) {
	ViewSetSpectrumColor( view, value );
	ViewFilledRectangle( view, 
		x - view->pseudocolor_x_radius, 
		y - view->pseudocolor_y_radius,
		x + view->pseudocolor_x_radius, 
		y + view->pseudocolor_y_radius );
}


/***************************************************************************/

void ViewPenSize (View view, int size) {
	
	Pen(view->display, size);
	
}

/***************************************************************************/

void ViewAxes(View view) {
	
	LinePattern(view->display, SOLID);
	if (view->user_left < 0.0 && 0.0 < view->user_right)
		ViewLine(view, 0.0, view->user_top, 0.0, view->user_bottom);
	if (view->user_bottom < 0.0 && 0.0 < view->user_top)
		ViewLine(view, view->user_left, 0.0, view->user_right, 0.0);
	
	LinePattern(view->display, view->display->pattern);
	
}



void ViewArrowAxes(View view, double atx, double aty ) {
	
	
	
	if ( atx == AT_ZERO ) atx = 0.0;
	else if ( atx == AT_MIN ) atx = view->user_left;
	else if ( atx == AT_MAX ) atx = view->user_right;
	
	if ( aty == AT_ZERO ) aty = 0.0;
	else if ( aty == AT_MIN ) aty = view->user_bottom;
	else if ( aty == AT_MAX ) aty = view->user_top;
	
	ViewArrow(view, atx, aty, atx, view->user_top );
	ViewArrow(view, atx, aty, atx, view->user_bottom );
	ViewArrow(view, atx, aty, view->user_left, aty );
	//  ViewArrow(view, atx, aty, view->user_right, aty );
	
	DisplayArrow(view->display,
	       UserToDisplayX(view, atx), 
		   UserToDisplayY(view, aty),
		   UserToDisplayX(view, view->user_right ) + 
		   view->display->symbol_radius,  
		   UserToDisplayY(view, aty));
	
}



/***************************************************************************/

void ViewVerticalLine( View view, double x ) {
	ViewLine( view, x, view->user_top, x, view->user_bottom );
}

void ViewHorizontalLine( View view, double y ) {
	ViewLine( view, view->user_left, y, view->user_right, y );
}

/***************************************************************************/

void ViewXTick( View view, double x ) {
	double tick_length = (view->user_top - view->user_bottom ) / 50.0;
	ViewLine( view, x, - tick_length, x, tick_length );
}


/***************************************************************************/

void ViewGrid (View view) {
	
	double start, end, step, p;
	double    pwr; 
	
	LinePattern(view->display, DOT);
	
	/* Align grid on powers of 10 */
	pwr = log10(view->user_right - view->user_left);
	if ( pwr == floor( pwr ) ) pwr = floor(pwr) - 2;
	else pwr = floor(pwr) - 1;
	
	step = pow(10.0, pwr);
	start = ceil(view->user_left / step) * step;
	end =  floor(view->user_right / step) * step;
	for (p = start; p <= end; p += step)
		ViewLine(view, p, view->user_bottom, p, view->user_top);
	
	pwr = log10(view->user_top - view->user_bottom);
	step = pow(10.0, pwr);
	if ( ( view->user_top - view->user_bottom ) / step > 10.0 ) pwr++;
	step = pow(10.0, pwr);
	
	start = ceil(view->user_bottom / step) * step;
	end = floor(view->user_top / step) * step;
	for (p = start; p <= end; p += step)
		ViewLine(view, view->user_left, p, view->user_right, p);
	
	ViewAxes(view);
	
}

/***************************************************************************/

void ViewSlash (View view) {
	
	Line(view->display, view->display_left, view->display_top,
		view->display_right, view->display_bottom);
	Line(view->display, view->display_left, view->display_bottom,
		view->display_right, view->display_top);
	
}

