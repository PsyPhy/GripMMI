///
/// Module:	GripMMI
/// 
///	Author:					J. McIntyre, PsyPhy Consulting
/// Initial release:		18 December 2014
/// Modification History:	see https://github.com/frenchjam/GripGroundMonitorClient
///
/// Copyright (c) 2014, 2015 PsyPhy Consulting
///

/// Methods for drawing the various graphs on the screen.

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

double lowerVisibilityLimit;
double upperVisibilityLimit;

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

	lowerVisibilityLimit = -20.0;
	upperVisibilityLimit =  100.0;

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
	LayoutSetDisplayEdgesRelative( stripchart_layout, 0.0, 0.075, 1.0, 0.99 );
	visibility_view = CreateView( stripchart_display );
	ViewSetDisplayEdgesRelative( visibility_view, 0.005, 0.01, 0.995, 0.07 );


}

void GripMMIDesktop::AdjustScrollSpan( void ) {
	double span = windowSpanSeconds[spanSelector->Value];
	double min, max;
	unsigned long	i;
	for ( i = 0; i < nFrames; i++ ) {
		if ( RealMarkerTime[i] != MISSING_DOUBLE ) {
			min = RealMarkerTime[i];
			break;
		}
	}
	for ( i = nFrames - 1; i >= 0; i-- ) {
		if ( RealMarkerTime[i] != MISSING_DOUBLE ) {
			max = RealMarkerTime[i];
			break;
		}
	}
	scrollBar->Minimum = floor( min ) + span / 10.0;
	scrollBar->Maximum = ceil( max );
	scrollBar->LargeChange = span;
	scrollBar->SmallChange = span / 10.0;

}
void GripMMIDesktop::MoveToLatest( void ) {
	scrollBar->Value = scrollBar->Maximum;
}


void GripMMIDesktop::RefreshGraphics( void ) {
		
	unsigned long first_sample;
	unsigned long last_sample;
	unsigned long index;

	fOutputDebugString( "Start RefreshGraphics().\n" );
	DisplayActivate( stripchart_display );
	Erase( stripchart_display );

	// Determine the time window based on the scroll bar position and the span slider.
	double last_instant = scrollBar->Value;
	double span = windowSpanSeconds[spanSelector->Value];
	double first_instant;

	// Find the indices into the arrays that correspond to the time window.
	for ( index = nFrames - 1; index > 0; index -- ) {
		if ( RealMarkerTime[index] != MISSING_DOUBLE && RealMarkerTime[index] <= last_instant ) break;
	}
	last_sample = index;
	last_instant = RealMarkerTime[last_sample];
	first_instant = last_instant - span;
	for ( index = index; index > 0; index -- ) {
		if ( RealMarkerTime[index] != MISSING_DOUBLE && RealMarkerTime[index] < first_instant ) break;
	}
	first_sample = index + 1;
	fOutputDebugString( "Data: %d to %d Graph: %lf to %lf Indices: %lu to %lu \n", scrollBar->Minimum, scrollBar->Maximum, first_instant, last_instant, first_sample, last_sample );

	// If the first and last index are the same, there is nothing to display,
	//  so just return to the caller.
	if ( last_sample <= first_sample ) return;


	switch ( graphCollectionComboBox->SelectedIndex ) {
	// Kinematics
	case 1:
		GraphManipulandumPositionComponent( X, LayoutViewN( stripchart_layout, 0 ), first_instant, last_instant, first_sample, last_sample );
		GraphManipulandumPositionComponent( Y, LayoutViewN( stripchart_layout, 1 ), first_instant, last_instant, first_sample, last_sample );
		GraphManipulandumPositionComponent( Z, LayoutViewN( stripchart_layout, 2 ), first_instant, last_instant, first_sample, last_sample );
		GraphAccelerationComponent( X, LayoutViewN( stripchart_layout, 3 ), first_instant, last_instant, first_sample, last_sample );
		GraphAccelerationComponent( Y, LayoutViewN( stripchart_layout, 4 ), first_instant, last_instant, first_sample, last_sample );
		GraphAccelerationComponent( Z, LayoutViewN( stripchart_layout, 5 ), first_instant, last_instant, first_sample, last_sample );
		break;
	// Summary
	case 0:
	default:
		GraphManipulandumPosition( LayoutViewN( stripchart_layout, 0 ), first_instant, last_instant, first_sample, last_sample );
		GraphManipulandumRotations( LayoutViewN( stripchart_layout, 1 ), first_instant, last_instant, first_sample, last_sample );
		GraphAcceleration( LayoutViewN( stripchart_layout, 2 ), first_instant, last_instant, first_sample, last_sample );
		GraphGripForce( LayoutViewN( stripchart_layout, 3 ), first_instant, last_instant, first_sample, last_sample );
		GraphLoadForce( LayoutViewN( stripchart_layout, 4 ), first_instant, last_instant, first_sample, last_sample );
		GraphCoP( LayoutViewN( stripchart_layout, 5 ), first_instant, last_instant, first_sample, last_sample );
		break;
	}
	GraphVisibility( visibility_view, first_instant, last_instant, first_sample, last_sample );
	OglSwap( stripchart_display );

	PlotManipulandumPosition( first_instant, last_instant, first_sample, last_sample );
	PlotCoP( first_instant, last_instant, first_sample, last_sample );

	fOutputDebugString( "Finish RefreshGraphics().\n" );

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
		ViewXYPlotAvailableDoubles( view, &RealMarkerTime[0], &ManipulandumPosition[0][i], start_frame, stop_frame, sizeof( *RealMarkerTime ), sizeof( *ManipulandumPosition ), MISSING_DOUBLE );
	}

}

void GripMMIDesktop::GraphManipulandumPositionComponent( int component, ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame ){
			
	char *title;

	ViewColor( view, GREY6 );
	ViewBox( view );
	ViewColor( view, BLACK );
	switch ( component ) {
	case X: title = "Manipulandum Position X "; break;
	case Y: title = "Manipulandum Position Y "; break;
	case Z: title = "Manipulandum Position Z "; break;
	default: title = "error";
	}
	ViewTitle( view, title, INSIDE_RIGHT, INSIDE_TOP, 0.0 );
	ViewSetXLimits( view, start_instant, stop_instant );
	if ( autoscaleCheckBox->Checked ) {
		ViewAutoScaleInit( view );
		ViewAutoScaleAvailableDoubles( view, &ManipulandumPosition[0][component], start_frame, stop_frame, sizeof( *ManipulandumPosition ), MISSING_DOUBLE );
	}
	else ViewSetYLimits( view, lowerPositionLimit, upperPositionLimit );
	ViewAxes( view );
	ViewSelectColor( view, component );
	ViewXYPlotAvailableDoubles( view, &RealMarkerTime[0], &ManipulandumPosition[0][component], start_frame, stop_frame, sizeof( *RealMarkerTime ), sizeof( *ManipulandumPosition ), MISSING_DOUBLE );
}
void GripMMIDesktop::GraphAccelerationComponent( int component, ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame ){
			
	char *title;

	ViewColor( view, GREY6 );
	ViewBox( view );
	ViewColor( view, BLACK );
	switch ( component ) {
	case X: title = "Manipulandum Acceleration X "; break;
	case Y: title = "Manipulandum Acceleration Y "; break;
	case Z: title = "Manipulandum Acceleration Z "; break;
	default: title = "error";
	}
	ViewTitle( view, title, INSIDE_RIGHT, INSIDE_TOP, 0.0 );

	ViewSetXLimits( view, start_instant, stop_instant );
	if ( autoscaleCheckBox->Checked ) {
		ViewAutoScaleInit( view );
		ViewAutoScaleAvailableDoubles( view, &Acceleration[0][component], start_frame, stop_frame, sizeof( *Acceleration ), MISSING_DOUBLE );
	}
	else ViewSetYLimits( view, lowerAccelerationLimit, upperAccelerationLimit );
	ViewAxes( view );
	ViewSelectColor( view, component );
	ViewXYPlotAvailableDoubles( view, &RealMarkerTime[0], &Acceleration[0][component], start_frame, stop_frame, sizeof( *RealMarkerTime ), sizeof( *Acceleration ), MISSING_DOUBLE );
}

void GripMMIDesktop::GraphManipulandumRotations( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame ){

	ViewColor( view, GREY6 );
	ViewBox( view );
	ViewColor( view, BLACK );
	ViewTitle( view, "Manipulandum Rotation ", INSIDE_RIGHT, INSIDE_TOP, 0.0 );

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
		ViewXYPlotAvailableDoubles( view, &RealMarkerTime[0], &ManipulandumRotations[0][i], start_frame, stop_frame, sizeof( *RealMarkerTime ), sizeof( *ManipulandumRotations ), MISSING_DOUBLE );
	}
}

void GripMMIDesktop::PlotManipulandumPosition( double start_instant, double stop_instant, int start_frame, int stop_frame ){

	::View view;

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
		ViewXYPlotAvailableDoubles( view, &RealMarkerTime[0], &LoadForce[0][i], start_frame, stop_frame, sizeof( *RealMarkerTime ), sizeof( *LoadForce ), MISSING_DOUBLE );
	}
	ViewSelectColor( view, i );
	ViewXYPlotAvailableDoubles( view, &RealMarkerTime[0], &LoadForceMagnitude[0], start_frame, stop_frame, sizeof( *RealMarkerTime ), sizeof( *LoadForceMagnitude ), MISSING_DOUBLE );

}
void GripMMIDesktop::GraphAcceleration( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame ) {

	ViewColor( view, GREY6 );
	ViewBox( view );
	ViewColor( view, BLACK );
	ViewTitle( view, "Acceleration ", INSIDE_RIGHT, INSIDE_TOP, 0.0 );

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
		ViewXYPlotAvailableDoubles( view, &RealMarkerTime[0], &Acceleration[0][i], start_frame, stop_frame, sizeof( *RealMarkerTime ), sizeof( *Acceleration ), MISSING_DOUBLE );
	}
}

void GripMMIDesktop::GraphGripForce( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame ) {

	ViewColor( view, GREY6 );
	ViewBox( view );
	ViewColor( view, BLACK );
	ViewTitle( view, "Grip Force ", INSIDE_RIGHT, INSIDE_TOP, 0.0 );

	ViewSetXLimits( view, start_instant, stop_instant );
	ViewSetYLimits( view, lowerGripLimit, upperGripLimit );
	ViewAxes( view );

	if ( autoscaleCheckBox->Checked ) {
		ViewAutoScaleInit( view );
		ViewAutoScaleAvailableDoubles( view, &GripForce[0], start_frame, stop_frame, sizeof( *GripForce ), MISSING_DOUBLE );
		ViewAutoScaleAvailableDoubles( view, &NormalForce[LEFT_ATI][0], start_frame, stop_frame, sizeof( *NormalForce[LEFT_ATI] ), MISSING_DOUBLE );
		ViewAutoScaleAvailableDoubles( view, &NormalForce[RIGHT_ATI][0], start_frame, stop_frame, sizeof( *NormalForce[RIGHT_ATI] ), MISSING_DOUBLE );
		ViewAutoScaleExpand( view, 0.01 );
	}

	ViewColor( view, atiColorMap[LEFT_ATI] );
	ViewXYPlotAvailableDoubles( view, &RealMarkerTime[0], &NormalForce[LEFT_ATI][0], start_frame, stop_frame, sizeof( *RealMarkerTime ), sizeof( *NormalForce[LEFT_ATI] ), MISSING_DOUBLE );
	ViewColor( view, atiColorMap[RIGHT_ATI] );
	ViewXYPlotAvailableDoubles( view, &RealMarkerTime[0], &NormalForce[RIGHT_ATI][0], start_frame, stop_frame, sizeof( *RealMarkerTime ), sizeof( *NormalForce[LEFT_ATI] ), MISSING_DOUBLE );
	ViewColor( view, GREEN );
	ViewXYPlotAvailableDoubles( view, &RealMarkerTime[0], &GripForce[0], start_frame, stop_frame, sizeof( *RealMarkerTime ), sizeof( *GripForce ), MISSING_DOUBLE );

}

void GripMMIDesktop::GraphVisibility( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame ) {

	ViewColor( view, GREY6 );
	ViewBox( view );
	ViewColor( view, BLACK );
	ViewTitle( view, "Visibility ", INSIDE_RIGHT, INSIDE_TOP, 0.0 );

	ViewSetXLimits( view, start_instant, stop_instant );
	ViewSetYLimits( view, lowerVisibilityLimit, upperVisibilityLimit );

	// Plot all the visibility traces in the same view;
	// Each marker is assigned a unique non-zero value when it is visible,
	//  such that the traces are spread out and grouped in the view.
//	for ( int mrk = 0; mrk < CODA_MARKERS; mrk++ ) {
//		ViewSelectColor( view, mrk );
//		ViewScatterPlotAvailableDoubles( view, SYMBOL_FILLED_SQUARE, &RealMarkerTime[0], &MarkerVisibility[0][mrk], start_frame, stop_frame, sizeof( *RealMarkerTime ), sizeof( *MarkerVisibility ), MISSING_DOUBLE );
//	}
	ViewColor( view, BLACK );
	ViewScatterPlotAvailableDoubles( view, SYMBOL_FILLED_SQUARE, &RealMarkerTime[0], &PacketReceived[0],  start_frame, stop_frame, sizeof( *RealMarkerTime ), sizeof( *PacketReceived ), MISSING_DOUBLE );
	ViewColor( view, RED );
	ViewScatterPlotAvailableDoubles( view, SYMBOL_FILLED_SQUARE, &RealMarkerTime[0], &ManipulandumVisibility[0],  start_frame, stop_frame, sizeof( *RealMarkerTime ), sizeof( *ManipulandumVisibility ), MISSING_DOUBLE );
	ViewColor( view, GREEN );
	ViewScatterPlotAvailableDoubles( view, SYMBOL_FILLED_SQUARE, &RealMarkerTime[0], &FrameVisibility[0],  start_frame, stop_frame, sizeof( *RealMarkerTime ), sizeof( *FrameVisibility ), MISSING_DOUBLE );
	ViewColor( view, BLUE );
	ViewScatterPlotAvailableDoubles( view, SYMBOL_FILLED_SQUARE, &RealMarkerTime[0], &WristVisibility[0],  start_frame, stop_frame, sizeof( *RealMarkerTime ), sizeof( *WristVisibility ), MISSING_DOUBLE );
//	ViewHorizontalLine( view, 3 );

}

void GripMMIDesktop::GraphCoP( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame ){

	ViewColor( view, GREY6 );
	ViewBox( view );
	ViewColor( view, BLACK );
	ViewTitle( view, "Center of Pressure ", INSIDE_RIGHT, INSIDE_TOP, 0.0 );

	ViewSetXLimits( view, start_instant, stop_instant );
	ViewSetYLimits( view, lowerCopLimit, upperCopLimit );
	ViewAxes( view );
	ViewHorizontalLine( view,  0.01 );
	ViewHorizontalLine( view, -0.01 );
		
	for ( int ati = 0; ati < 2; ati++ ) {
		for ( int i = X; i <= Z; i++ ) {
			ViewSelectColor( view, 3 * ati + i );
			ViewXYPlotClippedDoubles( view, &RealMarkerTime[0], &CenterOfPressure[ati][0][i], start_frame, stop_frame, sizeof( *RealMarkerTime ), sizeof( *CenterOfPressure[ati] ), MISSING_DOUBLE );
		}
	}
}

void GripMMIDesktop::PlotCoP( double start_instant, double stop_instant, int start_frame, int stop_frame ){

	::View view;

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
