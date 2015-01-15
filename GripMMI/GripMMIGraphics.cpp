// GripMMIGraphics.cpp : methods for drawing the various graphs on the screen.

#include "stdafx.h"
#include <Windows.h>

#include "GripMMIDesktop.h"

#include "..\Useful\Useful.h"
#include "..\Useful\fOutputDebugString.h"
#include "..\PsyPhy2dGraphicsLib\OglDisplayInterface.h"
#include "..\PsyPhy2dGraphicsLib\OglDisplay.h"
#include "..\PsyPhy2dGraphicsLib\Graphics.h"
#include "..\PsyPhy2dGraphicsLib\Views.h"
#include "..\PsyPhy2dGraphicsLib\Layouts.h"
#include "..\Grip\DexAnalogMixin.h"

using namespace GripMMI;

double lowerPositionLimit;
double upperPositionLimit;

double lowerRotationLimit;
double upperRotationLimit;

double lowerPositionLimitSpecific[3];
double upperPositionLimitSpecific[3];

double lowerForceLimit;
double upperForceLimit;
double lowerGripLimit;
double upperGripLimit;

double lowerAccelerationLimit;
double upperAccelerationLimit;

double lowerCopLimit;
double upperCopLimit;

// Define the pairs for each phase plot.
static struct {
	int abscissa;
	int ordinate;
} pair[PHASEPLOTS] = { {Z,Y}, {X,Y}, {X,Z} };

static int  atiColorMap[N_FORCE_TRANSDUCERS] = { CYAN, MAGENTA };

::View phase_view[PHASEPLOTS];
::Display phase_display[PHASEPLOTS];

void GripMMIDesktop::InitializeGraphics( void ) {

	HWND	parent;

	// Set the limits for the various graphs.
	// For the moment they are constant.

	lowerPositionLimit = -500.0;
	upperPositionLimit =  750.0;

	lowerRotationLimit = - Pi;
	upperRotationLimit =   Pi;

	lowerPositionLimitSpecific[X] = -200.0;
	upperPositionLimitSpecific[X] =  600.0, 
		
	lowerPositionLimitSpecific[Y] = -100.0;
	upperPositionLimitSpecific[Y] =  700.0, 

	lowerPositionLimitSpecific[Z] = -800.0;
	upperPositionLimitSpecific[Z] =    0.0;

	lowerAccelerationLimit = -2.0;
	upperAccelerationLimit =  2.0;


	lowerForceLimit = -10.0;
	upperForceLimit =  10.0;

	lowerGripLimit =  -2.0;
	upperGripLimit =  40.0;

	lowerCopLimit =  -0.030;
	upperCopLimit =   0.030;

				
	// Each Display will draw into a defined subwindow. 
	// All this code creates the link between the Display object
	//  and the corresponding subwindow, setting the size of the 
	//  Display object to that of the window.
	// More of this should be done inside the routine that creates
	//  the Display, but for historical purposes it is done here.

	// XY (Frontal plane)
	parent = static_cast<HWND>(XYPlot->Handle.ToPointer());
	xy_display = phase_display[0] = CreateOglDisplay();
	SetOglWindowParent(  parent );
	DisplaySetSizePixels( xy_display, XYPlot->Size.Width, XYPlot->Size.Height );
	DisplaySetScreenPosition( xy_display, 0, 0 );
	DisplayInit( xy_display );
	Erase( xy_display );
	xy_view = phase_view[0] = CreateView( xy_display );
	ViewSetDisplayEdgesRelative( xy_view, 0.01, 0.01, 0.99, 0.99 );
	ViewSetEdges( xy_view, 0, 0, 1, 1 );
	ViewMakeSquare(xy_view);

	// ZY (sagittal plane)
	parent = static_cast<HWND>(ZYPlot->Handle.ToPointer());
	zy_display = phase_display[1] = CreateOglDisplay();
	SetOglWindowParent(  parent );
	DisplaySetSizePixels( zy_display, ZYPlot->Size.Width, ZYPlot->Size.Height );
	DisplaySetScreenPosition( zy_display, 0, 0 );
	DisplayInit( zy_display );
	Erase( zy_display );
	zy_view = phase_view[1] = CreateView( zy_display );
	ViewSetDisplayEdgesRelative( zy_view, 0.01, 0.01, 0.99, 0.99 );
	ViewSetEdges( zy_view, 0, 0, 1, 1 );
	ViewMakeSquare(zy_view);

	// Center of Pressur (CoP)
	parent = static_cast<HWND>(CoPPlot->Handle.ToPointer());
	cop_display = phase_display[2] = CreateOglDisplay();
	SetOglWindowParent(  parent );
	DisplaySetSizePixels( cop_display, CoPPlot->Size.Width, CoPPlot->Size.Height );
	DisplaySetScreenPosition( cop_display, 0, 0 );
	DisplayInit( cop_display );
	Erase( cop_display );
	cop_view = phase_view[2] = CreateView( cop_display );
	ViewSetDisplayEdgesRelative( cop_view, 0.01, 0.01, 0.99, 0.99 );
	ViewSetEdges( cop_view, 0, 0, 1, 1 );
	ViewMakeSquare(cop_view);

	// Strip Charts
	parent = static_cast<HWND>(StripCharts->Handle.ToPointer());
   	stripchart_display = CreateOglDisplay();
	SetOglWindowParent( parent );
	DisplaySetSizePixels( stripchart_display, StripCharts->Size.Width, StripCharts->Size.Height );
	DisplaySetScreenPosition( stripchart_display, 0, 0 );
	DisplayInit( stripchart_display );
	Erase( stripchart_display );

	// Create an array of Views that will be used to plot data in stripchart form.
	stripchart_layout = CreateLayout( stripchart_display, STRIPCHARTS, 1 );
	LayoutSetDisplayEdgesRelative( stripchart_layout, 0.0, 0.055, 1.0, 0.99 );
	visibility_view = CreateView( stripchart_display );
	ViewSetDisplayEdgesRelative( visibility_view, 0.005, 0.01, 0.995, 0.05 );


}

void GripMMIDesktop::AdjustGraphSpan( void ) {
	int span = windowSpan[spanSelector->Value];
	scrollBar->Maximum = nFrames;
	scrollBar->LargeChange = span;
	scrollBar->SmallChange = span / 10;
}
void GripMMIDesktop::MoveToLatest( void ) {
	scrollBar->Value = nFrames;
}


void GripMMIDesktop::RefreshGraphics( void ) {
		
	int first_sample = 0;
	int last_sample = nFrames - 1;
	if ( last_sample <= first_sample ) return;

	int last_instant = scrollBar->Value;
	int span = windowSpan[spanSelector->Value];
	int first_instant = last_instant - span;
	fOutputDebugString( "Start RefreshGraphics().\n" );
	fOutputDebugString( "Last Instant: %d Span: %d %d\n", last_instant, span, SPAN_VALUES );

	DisplayActivate( stripchart_display );
	Erase( stripchart_display );

	GraphManipulandumPosition( LayoutViewN( stripchart_layout, 0 ), first_instant, last_instant, first_sample, last_sample );
	GraphManipulandumRotations( LayoutViewN( stripchart_layout, 1 ), first_instant, last_instant, first_sample, last_sample );
	GraphAcceleration( LayoutViewN( stripchart_layout, 2 ), first_instant, last_instant, first_sample, last_sample );
	GraphGripForce( LayoutViewN( stripchart_layout, 3 ), first_instant, last_instant, first_sample, last_sample );
	GraphLoadForce( LayoutViewN( stripchart_layout, 4 ), first_instant, last_instant, first_sample, last_sample );
	GraphCoP( LayoutViewN( stripchart_layout, 5 ), first_instant, last_instant, first_sample, last_sample );
	GraphVisibility( visibility_view, first_instant, last_instant, first_sample, last_sample );
	OglSwap( stripchart_display );

	PlotManipulandumPosition( first_instant, last_instant, first_sample, last_sample );
	PlotCoP( first_instant, last_instant, first_sample, last_sample );

	fOutputDebugString( "Finish RefreshGraphics().\n" );

}

void GripMMIDesktop::ChangeGraphics( void ) {
	static int color = 0;
	static int y = 100;
	static int x = 100;
	OglActivate( zy_display );
	Erase( zy_display );
	Color( zy_display, color );
	color = (color + 1)%6;
	Line( zy_display, 10, 10, x, y );
	x -= 10;
	glprintf( x, y, 12.0, "TESTAGAIN" );
	Swap();
}

void GripMMIDesktop::KillGraphics( void ) {

	Close( cop_display );
	Close( xy_display );
	Close( zy_display );
	Close( stripchart_display );

}

void GripMMIDesktop::GraphManipulandumPosition( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame ){
			
	double range;

	ViewColor( view, GREY6 );
	ViewBox( view );
	ViewColor( view, BLACK );
	ViewTitle( view, "Manipulandum Position ", INSIDE_RIGHT, INSIDE_TOP, 0.0 );

	if ( start_frame < start_instant ) start_frame = start_instant;
	if ( stop_frame >= stop_instant ) stop_frame = stop_instant - 1;
	if ( stop_frame <= start_frame ) return;

	// Plot all 3 components of the manipulandum position in the same view;
	// The autoscaling is a bit complicated. I want each trace centered on its own mean
	//  but I want the range of values to be common to all three so that magnitudes of movement
	//  can be compared between X, Y and Z.
	ViewSetXLimits( view, start_instant, stop_instant );
	if ( autoscaleCheckBox->Checked ) {
		range = 0.0;
		for ( int i = X; i <= Z; i++ ) {
			ViewAutoScaleInit( view );
			ViewAutoScaleAvailableDoubles( view, &ManipulandumPosition[0][i], start_frame, stop_frame, sizeof( *ManipulandumPosition ), MISSING_DOUBLE );
			if ( ViewYRange( view ) > range ) range = ViewYRange( view );
		}
	}
	ViewAxes( view );
	for ( int i = X; i <= Z; i++ ) {
		ViewSelectColor( view, i );
		if ( autoscaleCheckBox->Checked ) {
			ViewAutoScaleInit( view );
			ViewAutoScaleAvailableDoubles( view, &ManipulandumPosition[0][i], start_frame, stop_frame, sizeof( *ManipulandumPosition ), MISSING_DOUBLE );
			ViewSetYRange( view, range );
		}
		else ViewSetYLimits( view, lowerPositionLimit, upperPositionLimit );
		ViewPlotAvailableDoubles( view, &ManipulandumPosition[0][i], start_frame, stop_frame, sizeof( *ManipulandumPosition ), MISSING_DOUBLE );
	}

}

void GripMMIDesktop::GraphManipulandumRotations( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame ){

	ViewColor( view, GREY6 );
	ViewBox( view );
	ViewColor( view, BLACK );
	ViewTitle( view, "Manipulandum Rotation ", INSIDE_RIGHT, INSIDE_TOP, 0.0 );

	if ( start_frame < start_instant ) start_frame = start_instant;
	if ( stop_frame >= stop_instant ) stop_frame = stop_instant - 1;
	if ( stop_frame <= start_frame ) return;

	// Plot all 3 components of the manipulandum position in the same view;
	ViewSetXLimits( view, start_instant, stop_instant );
	if ( autoscaleCheckBox->Checked ) {
		ViewAutoScaleInit( view );
		for ( int i = X; i <= Z; i++ ) ViewAutoScaleAvailableDoubles( view, &ManipulandumRotations[0][i], start_frame, stop_frame, sizeof( *ManipulandumRotations ), MISSING_DOUBLE );
		ViewAutoScaleExpand( view, 0.01 );
	}
	else ViewSetYLimits( view, lowerRotationLimit, upperRotationLimit );
	ViewAxes( view );
	for ( int i = X; i <= Z; i++ ) {
		ViewSelectColor( view, i );
		ViewPlotAvailableDoubles( view, &ManipulandumRotations[0][i], start_frame, stop_frame, sizeof( *ManipulandumRotations ), MISSING_DOUBLE );
	}
}

void GripMMIDesktop::PlotManipulandumPosition( double start_instant, double stop_instant, int start_frame, int stop_frame ){

	::View view;

	if ( start_frame < start_instant ) start_frame = start_instant;
	if ( stop_frame >= stop_instant ) stop_frame = stop_instant - 1;

	// Phase plots of Manipulandum position data.
	for ( int i = 0; i < PHASEPLOTS - 1; i++ ) {

		DisplayActivate( phase_display[i] );
		Erase( phase_display[i] );
		view = phase_view[i];
		ViewSetXLimits( view, lowerPositionLimitSpecific[pair[i].abscissa], upperPositionLimitSpecific[pair[i].abscissa] );
		ViewSetYLimits( view, lowerPositionLimitSpecific[pair[i].ordinate], upperPositionLimitSpecific[pair[i].ordinate] );
		ViewMakeSquare( view );
		ViewSelectColor( view, i );
		// ViewBox( view );
		if ( stop_frame > start_frame ) ViewXYPlotAvailableDoubles( view, &ManipulandumPosition[0][pair[i].abscissa], &ManipulandumPosition[0][pair[i].ordinate], start_frame, stop_frame, sizeof( *ManipulandumPosition ), sizeof( *ManipulandumPosition ), MISSING_DOUBLE );
		OglSwap( phase_display[i] );
	}
}

void GripMMIDesktop::GraphLoadForce( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame ) {
	
	int i;
	
	ViewColor( view, GREY6 );
	ViewBox( view );
	ViewColor( view, BLACK );
	ViewTitle( view, "Load Force ", INSIDE_RIGHT, INSIDE_TOP, 0.0 );

	if ( start_frame < start_instant ) start_frame = start_instant;
	if ( stop_frame >= stop_instant ) stop_frame = stop_instant - 1;
	if ( stop_frame <= start_frame ) return;

	// Plot all 3 components of the load force in the same view;
	ViewSetXLimits( view, start_instant, stop_instant );
	if ( autoscaleCheckBox->Checked ) {
		ViewAutoScaleInit( view );
		for ( int i = X; i <= Z; i++ ) ViewAutoScaleAvailableDoubles( view, &LoadForce[0][i], start_frame, stop_frame, sizeof( *LoadForce ), MISSING_DOUBLE );
		ViewAutoScaleAvailableDoubles( view, &LoadForceMagnitude[0], start_frame, stop_frame, sizeof( *LoadForceMagnitude ), MISSING_DOUBLE );
		ViewAutoScaleExpand( view, 0.01 );
	}
	else ViewSetYLimits( view, lowerForceLimit, upperForceLimit );
	ViewAxes( view );
	ViewHorizontalLine( view, 0.0 );
	if ( view->user_top > 4.0 ) ViewHorizontalLine( view, 4.0 );
	if ( view->user_bottom < -4.0 ) ViewHorizontalLine( view, -4.0 );
	for ( i = X; i <= Z; i++ ) {
		ViewSelectColor( view, i );
		ViewPlotAvailableDoubles( view, &LoadForce[0][i], start_frame, stop_frame, sizeof( *LoadForce ), MISSING_DOUBLE );
	}
	ViewSelectColor( view, i );
	ViewPlotAvailableDoubles( view, &LoadForceMagnitude[0], start_frame, stop_frame, sizeof( *LoadForceMagnitude ), MISSING_DOUBLE );

}
void GripMMIDesktop::GraphAcceleration( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame ) {

	ViewColor( view, GREY6 );
	ViewBox( view );
	ViewColor( view, BLACK );
	ViewTitle( view, "Acceleration ", INSIDE_RIGHT, INSIDE_TOP, 0.0 );

	if ( start_frame < start_instant ) start_frame = start_instant;
	if ( stop_frame >= stop_instant ) stop_frame = stop_instant - 1;
	if ( stop_frame <= start_frame ) return;

	// Plot all 3 components of the acceleration in a single view;
	ViewSetXLimits( view, start_instant, stop_instant );
	if ( autoscaleCheckBox->Checked ) {
		ViewAutoScaleInit( view );
		for ( int i = X; i <= Z; i++ ) ViewAutoScaleAvailableDoubles( view, &Acceleration[0][i], start_frame, stop_frame, sizeof( *Acceleration ), MISSING_DOUBLE );
		ViewAutoScaleExpand( view, 0.01 );
	}
	else ViewSetYLimits( view, lowerAccelerationLimit, upperAccelerationLimit );
	ViewAxes( view );	
	for ( int i = 0; i < 3; i++ ) {
		ViewSelectColor( view, i );
		ViewPlotAvailableDoubles( view, &Acceleration[0][i], start_frame, stop_frame, sizeof( *Acceleration ), MISSING_DOUBLE );
	}
}

void GripMMIDesktop::GraphGripForce( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame ) {

	ViewColor( view, GREY6 );
	ViewBox( view );
	ViewColor( view, BLACK );
	ViewTitle( view, "Grip Force ", INSIDE_RIGHT, INSIDE_TOP, 0.0 );

	if ( start_frame < start_instant ) start_frame = start_instant;
	if ( stop_frame >= stop_instant ) stop_frame = stop_instant - 1;
	if ( stop_frame <= start_frame ) return;

	ViewSetXLimits( view, start_instant, stop_instant );
	ViewSetYLimits( view, lowerGripLimit, upperGripLimit );
	ViewAxes( view );

	if ( autoscaleCheckBox->Checked ) {
		ViewAutoScaleInit( view );
		ViewAutoScaleAvailableFloats( view, &GripForce[0], start_frame, stop_frame, sizeof( *GripForce ), MISSING_FLOAT );
		ViewAutoScaleAvailableFloats( view, &NormalForce[LEFT_ATI][0], start_frame, stop_frame, sizeof( *NormalForce[LEFT_ATI] ), MISSING_FLOAT );
		ViewAutoScaleAvailableFloats( view, &NormalForce[RIGHT_ATI][0], start_frame, stop_frame, sizeof( *NormalForce[RIGHT_ATI] ), MISSING_FLOAT );
		ViewAutoScaleExpand( view, 0.01 );
	}

	ViewColor( view, atiColorMap[LEFT_ATI] );
	ViewPlotAvailableFloats( view, &NormalForce[LEFT_ATI][0], start_frame, stop_frame, sizeof( *NormalForce[LEFT_ATI] ), MISSING_FLOAT );
	ViewColor( view, atiColorMap[RIGHT_ATI] );
	ViewPlotAvailableFloats( view, &NormalForce[RIGHT_ATI][0], start_frame, stop_frame, sizeof( *NormalForce[LEFT_ATI] ), MISSING_FLOAT );
	ViewColor( view, GREEN );
	ViewPlotAvailableFloats( view, &GripForce[0], start_frame, stop_frame, sizeof( *GripForce ), MISSING_FLOAT );

}

void GripMMIDesktop::GraphVisibility( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame ) {

	ViewColor( view, GREY6 );
	ViewBox( view );
	ViewColor( view, BLACK );
	ViewTitle( view, "Visibility ", INSIDE_RIGHT, INSIDE_TOP, 0.0 );

	if ( start_frame < start_instant ) start_frame = start_instant;
	if ( stop_frame >= stop_instant ) stop_frame = stop_instant - 1;
	if ( stop_frame <= start_frame ) return;

	ViewSetXLimits( view, start_instant, stop_instant );
	ViewSetYLimits( view, lowerGripLimit, upperGripLimit );
	ViewSetYLimits( view, 0, 100 );
	ViewAxes( view );

	// Plot all the visibility traces in the same view;
	// Each marker is assigned a unique non-zero value when it is visible,
	//  such that the traces are spread out and grouped in the view.
	for ( int mrk = 0; mrk < CODA_MARKERS; mrk++ ) {
		ViewColor( view, mrk % 3 );
		ViewPlotAvailableChars( view, &MarkerVisibility[0][mrk], start_frame, stop_frame, sizeof( *MarkerVisibility ), MISSING_CHAR );
	}

	ViewColor( view, BLACK );
	ViewBoxPlotChars( view, &ManipulandumVisibility[0],  start_frame, stop_frame, sizeof( *ManipulandumVisibility ) );
	ViewColor( view, RED );
//	ViewHorizontalLine( view, 3 );

}

void GripMMIDesktop::GraphCoP( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame ){

	ViewColor( view, GREY6 );
	ViewBox( view );
	ViewColor( view, BLACK );
	ViewTitle( view, "Center of Pressure ", INSIDE_RIGHT, INSIDE_TOP, 0.0 );

	if ( start_frame < start_instant ) start_frame = start_instant;
	if ( stop_frame >= stop_instant ) stop_frame = stop_instant - 1;
	if ( stop_frame <= start_frame ) return;

	ViewSetXLimits( view, start_instant, stop_instant );
	ViewSetYLimits( view, lowerCopLimit, upperCopLimit );
	ViewAxes( view );
	ViewHorizontalLine( view,  0.01 );
	ViewHorizontalLine( view, -0.01 );
		
	for ( int ati = 0; ati < 2; ati++ ) {
		for ( int i = X; i <= Z; i++ ) {
			ViewSelectColor( view, 3 * ati + i );
			ViewPlotClippedDoubles( view, &CenterOfPressure[ati][0][i], start_frame, stop_frame, sizeof( *CenterOfPressure[ati] ) );
		}
	}
}

void GripMMIDesktop::PlotCoP( double start_instant, double stop_instant, int start_frame, int stop_frame ){

	::View view;

	if ( start_frame < start_instant ) start_frame = start_instant;
	if ( stop_frame >= stop_instant ) stop_frame = stop_instant - 1;

	DisplayActivate( cop_display );
	Erase( cop_display );
	view = cop_view;
	ViewSetXLimits( view, lowerCopLimit, upperCopLimit );
	ViewSetYLimits( view, lowerCopLimit, upperCopLimit );
	ViewMakeSquare( view );

	// Plot the history of CoPs within the selected time window.
	if ( stop_frame > start_frame ) {
		ViewColor( view, atiColorMap[RIGHT_ATI] );
		ViewScatterPlotAvailableDoubles( view, SYMBOL_FILLED_SQUARE, &CenterOfPressure[RIGHT_ATI][0][Z], &CenterOfPressure[RIGHT_ATI][0][Y], start_frame, stop_frame, sizeof( *CenterOfPressure[RIGHT_ATI] ), sizeof( *CenterOfPressure[RIGHT_ATI] ), MISSING_FLOAT );
		ViewColor( view, atiColorMap[LEFT_ATI] );
		ViewScatterPlotAvailableDoubles( view, SYMBOL_FILLED_SQUARE, &CenterOfPressure[LEFT_ATI][0][Z], &CenterOfPressure[LEFT_ATI][0][Y], start_frame, stop_frame, sizeof( *CenterOfPressure[LEFT_ATI] ), sizeof( *CenterOfPressure[0] ), MISSING_FLOAT );
	}

	// If we are live, plot the current CoP.
	if ( dataLiveCheckbox->Checked ) {	
		ViewSetColor( view, RED );
		ViewFilledCircle( view, CenterOfPressure[0][stop_frame][Z], CenterOfPressure[0][stop_frame][Y], 0.0025 );
		ViewSetColor( view, BLUE );
		ViewFilledCircle( view, CenterOfPressure[1][stop_frame][Z], CenterOfPressure[1][stop_frame][Y], 0.0025 );
	}

	// Plot the critical region for a centered grip.
	ViewSetColor( view, GREY6 );
	ViewCircle( view, 0.0, 0.0, 0.010 );
	ViewSetColor( view, GREY6 );
	ViewCircle( view, 0.0, 0.0, 0.020 );
	OglSwap( cop_display );

}
