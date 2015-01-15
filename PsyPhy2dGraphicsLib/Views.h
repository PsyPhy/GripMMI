/*****************************************************************************/
/*                                                                           */
/*                                 Views.h                                   */
/*                                                                           */
/*****************************************************************************/

/* (c) Copyright 1994-1995, 2014-2015 PsyPhy Consulting. All Rights Reserved. */

/*

	A graphics library that faciliates transformations from user to screen
	coordinates. It is 'object oriented' before object-oriented C and C++
	were available.
	
*/

/*
	Transformation from user coordinates to device coordinates.
*/

#ifndef	_Views_
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _view {

  float	display_left;
  float	display_right;
  float	display_top;
  float	display_bottom;

  double	user_left;
  double	user_right;
  double	user_top;
  double	user_bottom;

  /* For pseudo-color depth displays. */
  double  user_min_depth;
  double  user_max_depth;

  double  pseudocolor_x_radius;
  double  pseudocolor_y_radius;

  double	x_factor;
  double	y_factor;

  int		x_sense;
  int		y_sense;

  Display	display;

  void *next;
	
} *View;

#define UserToDisplayX(v,x) 	(v->display_left + \
				 (float)((x - v->user_left) * v->x_factor))
#define UserToDisplayY(v,y)	(v->display_bottom + \
				 (float)((y - v->user_bottom) * v->y_factor))

#define ViewUserToDisplayX(v,x) UserToDisplayX(v,x)
#define ViewUserToDisplayY(v,x) UserToDisplayY(v,x)

#define	ViewUserToDisplayOffsetX(v, w) ((float)(w * v->x_factor))
#define	ViewUserToDisplayOffsetY(v, h) ((float)(h * v->y_factor))

#define	ViewUserToDisplayWidth(v, w) ((float) fabs(w * v->x_factor))
#define	ViewUserToDisplayHeight(v, h) ((float) fabs(h * v->y_factor))
#define	ViewUserToDisplayDistance(v, d) ((float) fabs(d * v->y_factor))

#define ViewDisplayWidth(v) 	(abs(v->display_right - v->display_left))
#define ViewDisplayHeight(v) 	(abs(v->display_bottom - v->display_top))

#define ViewLeft(v)			 	(v->user_left)
#define ViewRight(v)		 	(v->user_right)
#define ViewTop(v)			 	(v->user_top)
#define ViewBottom(v)		 	(v->user_bottom)

#define ViewWidth(v) 	(fabs(v->user_right - v->user_left))
#define ViewHeight(v) 	(fabs(v->user_bottom - v->user_top))

View CreateView (Display display);
void DestroyViews ( void );

void ViewInit (View view);
void ViewComputeFactors (View view);
void ViewDescribe (View view);
void ViewSetDisplayEdges (View view, int left, int bottom, int right, int top);
void ViewSetDisplayEdgesRelative (View view, double left, double bottom, 
				  double right, double top);
void ViewShiftRelative ( View view, double x, double y );

View ViewDisplayPointInside( View view, int x, int y );

void ViewSetEdges (View view, double left, double bottom, double right, double top);
void ViewSetXLimits (View view, double left, double right );
void ViewSetYLimits (View view, double left, double right );

double ViewYRange( View view );
void ViewSetYRange( View view, double range );
void ViewAdjustYRange( View view, double factor );

void ViewSetDepthLimits( View view, double min, double max );
#define ViewSetPseudoColorLimits( v, min, max ) ViewSetDepthLimits( v, min, max )
void ViewSetPseudoColorPixels( View view, int x_pixels, int y_pixels );
void ViewPlotPseudoColor( View view, double x, double y, double value );

void ViewCenterZeroY( View view );
void ViewMakeSquare (View view);


void ViewErase (View view);

void ViewBox (View view);
void ViewSlash (View view);
void ViewLineStyle (View view, int style);
void ViewLinePattern (View view, int pattern);
void ViewAlu (View view, int alu);

void ViewColor (View view, int color);
#define ViewSetColor( v, c ) ViewColor( v, c )
void ViewSelectColor (View view, int color);
void ViewSetSpectrumColor( View view, double value );


void ViewPenSize (View view, int size);

void ViewAxes(View view);

#define AT_ZERO -9999.99
#define AT_MIN	-8888.88
#define AT_MAX	-7777.77
void ViewArrowAxes(View view, double atx, double aty );

void ViewGrid (View view);
void ViewVerticalLine( View view, double x );
void ViewHorizontalLine( View view, double y );
void ViewXTick( View view, double x );

void ViewPoint (View view, double x, double y);
void ViewMoveTo (View view, double x, double y);
void ViewLineTo (View view, double x, double y);
void ViewLine (View view, double from_x, double from_y, double to_x, double to_y);

void ViewRectangle (View view, double x1, double y1, double x2, double y2);
void ViewFilledRectangle (View view, double x1, double y1, double x2, double y2);
void ViewFilledCircle (View view, double x, double y, double radius );
void ViewCircle (View view, double x, double y, double radius );
void ViewSymbol (View view, double x, double y, int symbol );
void ViewArrow (View view, double from_x, double from_y, double to_x, double to_y);


void ViewStartPolygon( View view );
void ViewAddVertex ( View view, double x, double y );
void ViewOutlinePolygon( View view );
void ViewFillPolygon( View view );

void ViewText (View view, char *string, double x, double y, double dir);

void ViewTitle (View view, char *string, int x, int y, double dir);

void ViewLabel (View view, char *string,
		double x, double y, double dir, 
		int x_justify, int y_justify );




/* Function Plots */

void ViewPlotLine (View view, double slope, double intercept);
void ViewPlotLineYX( View view, double slope, double intercept);
void ViewPlotEllipse (View view, double matrix[2][2], double scale, double center[2] );
void ViewPlotSquaredEllipse (View view, double matrix[2][2], double scale, double center[2] );
void ViewPlotFilledEllipse (View view, double matrix[2][2], double scale, double center[2] );
void ViewPlotSpectrumEllipse (View view, double matrix[2][2], double scale, double center[2],
                              double (*fn)( void *ptr, double angle  ), void *ptr );
void ViewPlotSpectrumCircle (View view, double x, double y, double radius, 
                             double (*fn)( void *ptr, double angle ), void *ptr );
void ViewPlotPolynomial (View view, double p[], int n );

/* Array Plots */

void ViewsSetArraySkip( int N );


void ViewAutoScaleInit( View view );
void ViewAutoScaleShorts ( View view, short *array, 

			   unsigned start, unsigned end, unsigned size );

void ViewAutoScaleInts ( View view, int *array, 

			   unsigned start, unsigned end, unsigned size );

void ViewAutoScaleFloats ( View view, float *array, 
			   unsigned start, unsigned end, unsigned size );
void ViewAutoScaleAvailableFloats ( View view, float *array, 

				     unsigned start, unsigned end, unsigned size,

				     float NA );

void ViewAutoScaleDoubles ( View view, double *array, 
			    unsigned start, unsigned end, unsigned size );
void ViewAutoScaleAvailableDoubles ( View view, double *array, 
				unsigned start, unsigned end, 
				unsigned size, double NA );
void ViewAutoScaleMostDoubles ( View view,
					double *array, 
					unsigned start, 
					unsigned end, 
					unsigned size,
					double NA,
          double exclude );

void ViewAutoScaleExpand( View view, double fraction );
void ViewAutoScaleSetInterval( View view, double interval );

void ViewPlotChars ( View view, char *array, 
		     unsigned start, unsigned end, unsigned size );
void ViewPlotAvailableChars ( View view, char *array, 
							 unsigned start, unsigned end, 
							 unsigned size, int NA );


void ViewPlotShorts ( View view, short *array, 
		      unsigned start, unsigned end, unsigned size );
void ViewPlotInts ( View view, int *array, 
		      unsigned start, unsigned end, unsigned size );
void ViewPlotFloats (View view, float *array, 
		     unsigned start, unsigned end, unsigned size );
void ViewPlotClippedFloats ( View view, float *array, 
		     unsigned start, unsigned end, unsigned size );
void ViewPlotAvailableFloats ( View view, float *array, 
				unsigned start, unsigned end, 
				unsigned size, float NA );

void ViewPlotFloatsDashed (View view, float *array, 
			   unsigned start, unsigned end, 
			   unsigned size, double on_size, double off_size );

void ViewPlotDoubles ( View view, double *array, 
		       unsigned start, unsigned end, unsigned skip, unsigned size );
void ViewPlotClippedDoubles ( View view, double *array, 
		       unsigned start, unsigned end, unsigned size, double NA );

void ViewPlotAvailableDoubles ( View view, double *array, 
				unsigned start, unsigned end, 
				unsigned size, double NA );
void ViewPointPlotAvailableDoubles ( View view, double *array, 
				unsigned start, unsigned end, 
				unsigned size, double NA );


void ViewBoxPlotChars (View view, char *array, 
			unsigned start, unsigned end, unsigned size );
void ViewBoxPlotShorts (View view, short *array, 
			unsigned start, unsigned end, unsigned size );
void ViewBoxPlotInts (View view, int *array, 
			unsigned start, unsigned end, unsigned size );
void ViewBoxPlotFloats (View view, float *array, 
			unsigned start, unsigned end, unsigned size );


void ViewXYPlotInts (View view, int *xarray, int *yarray, 
		     unsigned start, unsigned end, 
		     unsigned xsize, unsigned ysize);
void ViewXYPlotFloats (View view, float *xarray, float *yarray, 
		       unsigned start, unsigned end, 
		       unsigned xsize, unsigned ysize);
void ViewXYPlotDoubles (View view, double *xarray, double *yarray, 
			unsigned start, unsigned end, unsigned skip,
			unsigned xsize, unsigned ysize);
void ViewXYPlotAvailableFloats (View view, float *xarray, float *yarray, 
				unsigned start, unsigned end, 
				unsigned xsize, unsigned ysize, 
				float na);
void ViewXYPlotAvailableDoubles (View view, double *xarray, double *yarray, 
				 unsigned start, unsigned end, 
				 unsigned xsize, unsigned ysize, 
				 double na);
						   
void ViewScatterPlotFloats (View view, int symbol, 
			    float *xarray, float *yarray, 
			    unsigned start, unsigned end, 
			    unsigned xsize, unsigned ysize);

void ViewScatterPlotAvailableFloats (View view, int symbol, 
				     float *xarray, float *yarray, 
				     unsigned start, unsigned end, 
				     unsigned xsize, unsigned ysize, 
				     float NA);


void ViewScatterPlotInts (View view, int symbol,
			  int *xarray, int *yarray, 
			  unsigned start, unsigned end, 
			  unsigned xsize, unsigned ysize);
void ViewScatterPlotDoubles (View view, int symbol,
			     double *xarray, double *yarray, 
			     unsigned start, unsigned end, 
			     unsigned xsize, unsigned ysize);
void ViewScatterPlotAvailableDoubles (View view, int symbol,
				     double *xarray, double *yarray, 
				     unsigned start, unsigned end, 
				     unsigned xsize, unsigned ysize,
				     double NA );

#ifdef __cplusplus
}
#endif

#define	_Views_

#endif

