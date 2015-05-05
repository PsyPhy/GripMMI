///
/// Module:	GripMMI
/// 
///	Author:					J. McIntyre, PsyPhy Consulting
/// Initial release:		18 December 2014
/// Modification History:	see https://github.com/PsyPhy/GripMMI
///
/// Copyright (c) 2014, 2015 PsyPhy Consulting
///

/// Methods for drawing the various graphs on the screen.

#include "stdafx.h"
#include <Windows.h>

#include "GripMMIDesktop.h"

#include "..\Useful\Useful.h"
#include "..\Useful\fOutputDebugString.h"
#include "..\Useful\fMessageBox.h"

// We make use of a package of plotting routines that I have had around for decades.
#include "..\PsyPhy2dGraphicsLib\OglDisplayInterface.h"
#include "..\PsyPhy2dGraphicsLib\OglDisplay.h"
#include "..\PsyPhy2dGraphicsLib\Graphics.h"
#include "..\PsyPhy2dGraphicsLib\Views.h"
#include "..\PsyPhy2dGraphicsLib\Layouts.h"

// Some of the raw data needs to be processed in the same way as the GRIP hardware does it.
#include "..\Grip\DexAnalogMixin.h"

using namespace GripMMI;

// Local constants defining the ranges of various plots.

double	lowerPositionLimit = -500.0;
double	upperPositionLimit =  750.0;

double	lowerRotationLimit = -Pi;
double	upperRotationLimit =  Pi;

double	lowerAccelerationLimit = -2.0;
double	upperAccelerationLimit =  2.0;

double	lowerVisibilityLimit = -20.0;
double	upperVisibilityLimit =  100.0;

double	lowerForceLimit = -10.0;
double	upperForceLimit =  10.0;

double	lowerGripLimit =  -2.0;
double	upperGripLimit =  40.0;

double	lowerCopLimit =  -0.030;
double	upperCopLimit =   0.030;

// It is convenient to have an array of position ranges
//  that are specific to X, Y and Z.
// These are initialized by GripMMIDesktop::InitializeGraphics().
double lowerPositionLimitSpecific[3];
double upperPositionLimitSpecific[3];

// Define the pairs for each phase plot.
static struct {
	int abscissa;
	int ordinate;
} pair[PHASEPLOTS] = { {X,Y}, {Z,Y}, {X,Z} };

static int  atiColorMap[N_FORCE_TRANSDUCERS] = { CYAN, MAGENTA };

//  It is useful to group the phase plots into arrays so that they can be processed in a loop.
::View phase_view[PHASEPLOTS];
::Display phase_display[PHASEPLOTS];

// Initialize the objects used to plot the data on the screen.
void GripMMIDesktop::InitializeGraphics( void ) {

	HWND	parent;

	// It is convenient to have an array of position ranges
	//  that are specific to X, Y and Z.
	lowerPositionLimitSpecific[X] = -200.0;
	upperPositionLimitSpecific[X] =  600.0, 
		
	lowerPositionLimitSpecific[Y] = -100.0;
	upperPositionLimitSpecific[Y] =  700.0, 

	lowerPositionLimitSpecific[Z] = -800.0;
	upperPositionLimitSpecific[Z] =    0.0;

	// Each Display will draw into a defined subwindow. 
	// The View code defines 'Displays' as regions of a window and
	// 'Views' as sub-regions with define limits in user coordinates.

	// All the code below creates the link between the Display object
	//  and the corresponding Windows pane, setting the size of the 
	//  Display object to that of the pane.
	// More of this should be done inside the routine that creates
	//  the Display, but for historical purposes it is done here.

	// Then the Views are defined with respect to each Display and the 
	//  limits in user coordinates are initialized. 

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

	// Create a Layout (an array of Views) that will be used to plot data in stripchart form.
	stripchart_layout = CreateLayout( stripchart_display, STRIPCHARTS, 1 );
	LayoutSetDisplayEdgesRelative( stripchart_layout, 0.0, 0.065, 1.0, 1.0 );
	visibility_view = CreateView( stripchart_display );
	ViewSetDisplayEdgesRelative( visibility_view, 0.005, 0.0, 0.995, 0.06 );

	// Create a specific view for displaying detailed visibility information.
	// It is not part of the stripchart Layout because I want it to be smaller in height
	//  than the rest of the stripcharts.
	detailed_visibility_layout = CreateLayout( stripchart_display, 4, 1 );
	LayoutSetDisplayEdgesRelative( detailed_visibility_layout, 0.0, 0.065, 1.0, 1.0 );

}

// The GUI has a scroll bar allowing one to look back at different parts of the data and
// a slider that determines the time span of the data window. Here we determine the time span 
// of the available data, set the limits on the scroll bar accordingly, and show the limits
// in clock format under the scroll bar.
void GripMMIDesktop::AdjustScrollSpan( void ) {
	double min, max;
	int since_midnight, hour, minute, second;
	int day_last, day_first;
	char label[32], modifier[32];
	unsigned long	i;

	// The time span of data to plot is determined by the slider.
	double span = windowSpanSeconds[spanSelector->Value];

	// Find the time window of the available data packets.
	// The global array RealMarkerTime[] has been filled previously.
	min = 0.0;
	for ( i = 0; i < nFrames; i++ ) {
		if ( RealMarkerTime[i] != MISSING_DOUBLE ) {
			min = RealMarkerTime[i];
			break;
		}
	}
	max = span;
	for ( i = nFrames - 1; i >= 0; i-- ) {
		if ( RealMarkerTime[i] != MISSING_DOUBLE ) {
			max = RealMarkerTime[i];
			break;
		}
	}
	// Adjust the behavior of the scroll bar depending on the selected 
	// time span of the data window. A large step moves a full window
	// width, a small step moves 1/10th of the window.
	scrollBar->LargeChange = span;
	scrollBar->SmallChange = span / 10.0;
	// Set the scroll bar limits to match the limits of the available data.
	// Note that you cannot not reach the max value of the scroll bar with the user
	// controls. Best you can do is to get within LargeChange of the maximum. 
	// We extend the range of the scroll bar by that value so that one can
	//  reach the end of the data.
	int top = ceil( max ) + scrollBar->LargeChange;
	int bottom = floor( min );
	if ( bottom >= floor( max ) ) bottom = floor( max );
	scrollBar->Maximum = top;
	scrollBar->Minimum = bottom;

	// Label the two ends of the scroll bar.
	day_last = (int) ceil(( max + TimebaseOffset )) / (24 * 60 * 60);
	since_midnight = ((int) ceil( max ) + TimebaseOffset ) % (24 * 60 * 60);
	hour = since_midnight / (60 * 60);
	minute = (since_midnight % (60 * 60)) / 60;
	second = (since_midnight % 60);
	sprintf( label, "%02d:%02d:%02d", hour, minute, second  );
	latestTextBox->Text = gcnew String( label );

	day_first = (int) floor(( min + TimebaseOffset )) / (24 * 60 * 60);
	since_midnight = ((int) floor( min ) + TimebaseOffset ) % (24 * 60 * 60);
	hour = since_midnight / (60 * 60);
	minute = (since_midnight % (60 * 60)) / 60;
	second = (since_midnight % 60);
	if ( day_last == day_first ) sprintf( modifier, "" );
	else sprintf( modifier, "J-%d", day_last - day_first );
	sprintf( label, "%02d:%02d:%02d %s", hour, minute, second, modifier  );
	earliestTextBox->Text = gcnew String( label );
}

// When the display is 'live' we want to be able to automatically position the scroll bar 
// so as to display the most recent data.
void GripMMIDesktop::MoveToLatest( void ) {
	double latest;
	for ( int i = nFrames - 1; i >= 0; i-- ) {
		if ( RealMarkerTime[i] != MISSING_DOUBLE ) {
			latest = RealMarkerTime[i];
			break;
		}
	}
	scrollBar->Value = ceil( latest );
}

// Here we do the actual work of plotting the strip charts and phase plots.
// It is assumed that the global data arrays have been filled. The time span
// of the plots is determined by the scroll bar and span slider.
void GripMMIDesktop::RefreshGraphics( void ) {
		
	int since_midnight, hour, minute, second;
	int day_last, day_first;
	char label[32], modifier[32];

	unsigned long first_sample;
	unsigned long last_sample;
	unsigned long index;

	fOutputDebugString( "Start RefreshGraphics().\n" );
	DisplayActivate( stripchart_display );
	Erase( stripchart_display );

	// Determine the time window, in seconds, based on the scroll bar position and the span slider.
	double last_instant = scrollBar->Value;
	double span = windowSpanSeconds[spanSelector->Value];
	double first_instant = last_instant - span;

	// Convert the earliest and latest instants to clock format and display them.
	day_last = (int) ceil( last_instant + TimebaseOffset ) / (24 * 60 * 60);
	since_midnight = ((int) ceil( last_instant + TimebaseOffset )) % (24 * 60 * 60);
	hour = since_midnight / (60 * 60);
	minute = (since_midnight % (60 * 60)) / 60;
	second = (since_midnight % 60);
	sprintf( label, "%02d:%02d:%02d", hour, minute, second  );
	rightLimitTextBox->Text = gcnew String( label );

	day_first = (int) floor( first_instant + TimebaseOffset ) / (24 * 60 * 60);
	since_midnight = ((int) floor( first_instant + TimebaseOffset )) % (24 * 60 * 60);
	hour = since_midnight / (60 * 60);
	minute = (since_midnight % (60 * 60)) / 60;
	second = (since_midnight % 60);
	if ( day_last == day_first ) sprintf( modifier, "" );
	else sprintf( modifier, "J-%d", day_last - day_first );
	sprintf( label, "%02d:%02d:%02d %s", hour, minute, second, modifier  );
	leftLimitTextBox->Text = gcnew String( label );

	// Find the indices into the arrays that correspond to the time window.
	for ( index = nFrames - 1; index > 0; index -- ) {
		if ( RealMarkerTime[index] != MISSING_DOUBLE && RealMarkerTime[index] <= last_instant ) break;
	}
	last_sample = index;
	for ( index = index; index > 0; index -- ) {
		if ( RealMarkerTime[index] != MISSING_DOUBLE && RealMarkerTime[index] < first_instant ) break;
	}
	first_sample = index + 1;
	// fOutputDebugString( "Data: %d to %d Graph: %lf to %lf Indices: %d to %d (%d)\n", scrollBar->Minimum, scrollBar->Maximum, first_instant, last_instant, first_sample, last_sample, (last_sample - first_sample) );

	// Subsample the data if there is a lot to be plotted.
	int step = 1;
	while ( ((last_sample - first_sample) / step) > MAX_PLOT_SAMPLES && step < (MAX_PLOT_STEP - 1) ) step++;
	// fOutputDebugString( "Plot step: %d\n", step );

	// The user can select different combinations of strip charts to plot by making a selection in a pull-down list.
	// The following code generates the different plots depending on the selection.
	switch ( graphCollectionComboBox->SelectedIndex ) {
	// Marker Visibility Plot
	case 2:
		GraphManipulandumPositionComponent( X, LayoutViewN( detailed_visibility_layout, 0 ), first_instant, last_instant, first_sample, last_sample, step );
		GraphManipulandumPositionComponent( Y, LayoutViewN( detailed_visibility_layout, 1 ), first_instant, last_instant, first_sample, last_sample, step );
		GraphManipulandumPositionComponent( Z, LayoutViewN( detailed_visibility_layout, 2 ), first_instant, last_instant, first_sample, last_sample, step );
		GraphVisibilityDetails( LayoutViewN( detailed_visibility_layout, 3 ), first_instant, last_instant, first_sample, last_sample, step );
		GraphVisibility( visibility_view, first_instant, last_instant, first_sample, last_sample, step );
		break;
	// Kinematics Plot
	case 1:
		GraphManipulandumPositionComponent( X, LayoutViewN( stripchart_layout, 0 ), first_instant, last_instant, first_sample, last_sample, step );
		GraphManipulandumPositionComponent( Y, LayoutViewN( stripchart_layout, 1 ), first_instant, last_instant, first_sample, last_sample, step );
		GraphManipulandumPositionComponent( Z, LayoutViewN( stripchart_layout, 2 ), first_instant, last_instant, first_sample, last_sample, step );
		GraphAccelerationComponent( X, LayoutViewN( stripchart_layout, 3 ), first_instant, last_instant, first_sample, last_sample, step );
		GraphAccelerationComponent( Y, LayoutViewN( stripchart_layout, 4 ), first_instant, last_instant, first_sample, last_sample, step );
		GraphAccelerationComponent( Z, LayoutViewN( stripchart_layout, 5 ), first_instant, last_instant, first_sample, last_sample, step );
		GraphVisibility( visibility_view, first_instant, last_instant, first_sample, last_sample, step );
		break;
	// Summary Plot
	case 0:
	default:
		GraphManipulandumPosition( LayoutViewN( stripchart_layout, 0 ), first_instant, last_instant, first_sample, last_sample, step );
		GraphManipulandumRotations( LayoutViewN( stripchart_layout, 1 ), first_instant, last_instant, first_sample, last_sample, step );
		GraphAcceleration( LayoutViewN( stripchart_layout, 2 ), first_instant, last_instant, first_sample, last_sample, step );
		GraphGripForce( LayoutViewN( stripchart_layout, 3 ), first_instant, last_instant, first_sample, last_sample, step );
		GraphLoadForce( LayoutViewN( stripchart_layout, 4 ), first_instant, last_instant, first_sample, last_sample, step );
		GraphCoP( LayoutViewN( stripchart_layout, 5 ), first_instant, last_instant, first_sample, last_sample, step );
		GraphVisibility( visibility_view, first_instant, last_instant, first_sample, last_sample, step );
		break;
	}
	// The Views code requires a display swap to make the plots visible.
	OglSwap( stripchart_display );

	// Generate the phase plots.
	PlotManipulandumPosition( first_instant, last_instant, first_sample, last_sample, step );
	PlotCoP( first_instant, last_instant, first_sample, last_sample, step );

	fOutputDebugString( "Finish RefreshGraphics().\n" );

}

// Clean up resources allocated by the Views system.
void GripMMIDesktop::KillGraphics( void ) {

	Close( cop_display );
	Close( xy_display );
	Close( zy_display );
	Close( stripchart_display );

}

///
/// Plotting into Stripcharts
///

// The following routines all share a similar format:
//
//  Parameter 'view' is a pointer to a structure that maps data values to screen coordinates. Basically it references a rectangle on the screen.
//  Parameters 'start_instant' and 'stop_instant' determine the time window in seconds.
//  Parameters 'start_index', 'stop_index' determine the range of entries in the data arrays to be used for the graphs.
//  Parameter 'step' allows for sub-sampling of the data to reduce the number of points that are actually plotted.
//
// All of these routines assume that the data arrays are already filled with valid data.

// The plotting routines use pointers and byte sizes to allow one to plot, for instance, one component from an array 
// of vectors. The sizeof() macro is used to compute the distance in bytes between elements in the array.

// The plotting routines also take a "missing value" flag. Data that is set to this value will not be plotted. This
//  is used to mark breaks in the data stream.

// In the following method names, 'Graph...' refers to a stripchart, while 'Plot...' refers to a phase plot.

void GripMMIDesktop::GraphManipulandumPosition( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame, int step ){
			
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
		// Find the common range.
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
			// Autoscale each component to center each trace on its respective mean.
			ViewAutoScaleInit( view );
			ViewAutoScaleAvailableDoubles( view, &ManipulandumPosition[0][i], start_frame, stop_frame, sizeof( *ManipulandumPosition ), MISSING_DOUBLE );
			// But expand the Y limits so that all 3 components are plotted on a common scale.
			ViewSetYRange( view, range );
		}
		else {
			// Use the fixed limits.
			ViewSetYLimits( view, lowerPositionLimit, upperPositionLimit );
		}
		// Actually plot the data.
		ViewXYPlotAvailableDoubles( view, &RealMarkerTime[0], &ManipulandumPosition[0][i], start_frame, stop_frame, step, sizeof( *RealMarkerTime ), sizeof( *ManipulandumPosition ), MISSING_DOUBLE );
	}

}

void GripMMIDesktop::GraphManipulandumPositionComponent( int component, ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame, int step ){
			
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
	ViewXYPlotAvailableDoubles( view, &RealMarkerTime[0], &ManipulandumPosition[0][component], start_frame, stop_frame, step, sizeof( *RealMarkerTime ), sizeof( *ManipulandumPosition ), MISSING_DOUBLE );
}

void GripMMIDesktop::GraphAccelerationComponent( int component, ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame, int step ){
			
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
	ViewXYPlotAvailableDoubles( view, &RealMarkerTime[0], &Acceleration[0][component], start_frame, stop_frame, step, sizeof( *RealMarkerTime ), sizeof( *Acceleration ), MISSING_DOUBLE );
}

void GripMMIDesktop::GraphManipulandumRotations( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame, int step ){

	ViewColor( view, GREY6 );
	ViewBox( view );
	ViewColor( view, BLACK );
	ViewTitle( view, "Manipulandum Rotation ", INSIDE_RIGHT, INSIDE_TOP, 0.0 );

	// Plot all 3 components of the manipulandum rotation in the same view;
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
		ViewXYPlotAvailableDoubles( view, &RealMarkerTime[0], &ManipulandumRotations[0][i], start_frame, stop_frame, step, sizeof( *RealMarkerTime ), sizeof( *ManipulandumRotations ), MISSING_DOUBLE );
	}
}


void GripMMIDesktop::GraphLoadForce( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame, int step ) {
	
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
	// Show zero load force and a +/- 4 Newton range.
	ViewHorizontalLine( view, 0.0 );
	if ( view->user_top > 4.0 ) ViewHorizontalLine( view, 4.0 );
	if ( view->user_bottom < -4.0 ) ViewHorizontalLine( view, -4.0 );
	for ( i = X; i <= Z; i++ ) {
		ViewSelectColor( view, i );
		ViewXYPlotAvailableDoubles( view, &RealMarkerTime[0], &LoadForce[0][i], start_frame, stop_frame, step, sizeof( *RealMarkerTime ), sizeof( *LoadForce ), MISSING_DOUBLE );
	}
	ViewSelectColor( view, i );
	ViewXYPlotAvailableDoubles( view, &RealMarkerTime[0], &LoadForceMagnitude[0], start_frame, stop_frame, step, sizeof( *RealMarkerTime ), sizeof( *LoadForceMagnitude ), MISSING_DOUBLE );

}
void GripMMIDesktop::GraphAcceleration( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame, int step ) {

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
		ViewXYPlotAvailableDoubles( view, &RealMarkerTime[0], &Acceleration[0][i], start_frame, stop_frame, step, sizeof( *RealMarkerTime ), sizeof( *Acceleration ), MISSING_DOUBLE );
	}
}

void GripMMIDesktop::GraphGripForce( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame, int step ) {

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
	ViewXYPlotAvailableDoubles( view, &RealMarkerTime[0], &NormalForce[LEFT_ATI][0], start_frame, stop_frame, step, sizeof( *RealMarkerTime ), sizeof( *NormalForce[LEFT_ATI] ), MISSING_DOUBLE );
	ViewColor( view, atiColorMap[RIGHT_ATI] );
	ViewXYPlotAvailableDoubles( view, &RealMarkerTime[0], &NormalForce[RIGHT_ATI][0], start_frame, stop_frame, step, sizeof( *RealMarkerTime ), sizeof( *NormalForce[LEFT_ATI] ), MISSING_DOUBLE );
	ViewColor( view, GREEN );
	ViewXYPlotAvailableDoubles( view, &RealMarkerTime[0], &GripForce[0], start_frame, stop_frame, step, sizeof( *RealMarkerTime ), sizeof( *GripForce ), MISSING_DOUBLE );

}

void GripMMIDesktop::GraphVisibility( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame, int step ) {

	ViewColor( view, GREY6 );
	ViewBox( view );
	ViewColor( view, BLACK );
	ViewTitle( view, "Visibility ", INSIDE_RIGHT, INSIDE_TOP, 0.0 );

	ViewSetXLimits( view, start_instant, stop_instant );
	ViewSetYLimits( view, lowerVisibilityLimit, upperVisibilityLimit );

	ViewColor( view, BLACK );
	ViewScatterPlotAvailableDoubles( view, SYMBOL_FILLED_SQUARE, &RealMarkerTime[0], &PacketReceived[0],  start_frame, stop_frame, step, sizeof( *RealMarkerTime ), sizeof( *PacketReceived ), MISSING_DOUBLE );
	ViewColor( view, RED );
	ViewScatterPlotAvailableDoubles( view, SYMBOL_FILLED_SQUARE, &RealMarkerTime[0], &ManipulandumVisibility[0],  start_frame, stop_frame, step, sizeof( *RealMarkerTime ), sizeof( *ManipulandumVisibility ), MISSING_DOUBLE );
	ViewColor( view, GREEN );
	ViewScatterPlotAvailableDoubles( view, SYMBOL_FILLED_SQUARE, &RealMarkerTime[0], &FrameVisibility[0],  start_frame, stop_frame, step, sizeof( *RealMarkerTime ), sizeof( *FrameVisibility ), MISSING_DOUBLE );
	ViewColor( view, BLUE );
	ViewScatterPlotAvailableDoubles( view, SYMBOL_FILLED_SQUARE, &RealMarkerTime[0], &WristVisibility[0],  start_frame, stop_frame, step, sizeof( *RealMarkerTime ), sizeof( *WristVisibility ), MISSING_DOUBLE );

}

void GripMMIDesktop::GraphVisibilityDetails( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame, int step ) {

	int mrk;

	ViewColor( view, GREY6 );
	ViewBox( view );
	ViewColor( view, BLACK );
	ViewTitle( view, "Marker Visibility ", INSIDE_RIGHT, INSIDE_TOP, 0.0 );

	ViewSetXLimits( view, start_instant, stop_instant );
	ViewSetYLimits( view, 0, 28 );

	ViewSetColor( view, GREY6 );
	for ( mrk = 1; mrk <= 8; mrk++ ) ViewHorizontalLine( view, mrk );
	for ( mrk = 11; mrk <= 14; mrk++ ) ViewHorizontalLine( view, mrk );
	for ( mrk = 17; mrk <= 24; mrk++ ) ViewHorizontalLine( view, mrk );

	// Plot all the visibility traces in the same view;
	// Each marker is assigned a unique non-zero value when it is visible,
	//  such that the traces are spread out and grouped in the view.
	for ( mrk = 0; mrk < CODA_MARKERS; mrk++ ) {
		ViewSelectColor( view, mrk );
		ViewScatterPlotAvailableDoubles( view, SYMBOL_FILLED_SQUARE, &RealMarkerTime[0], &MarkerVisibility[0][mrk], start_frame, stop_frame, step, sizeof( *RealMarkerTime ), sizeof( *MarkerVisibility ), MISSING_DOUBLE );
	}
}

void GripMMIDesktop::GraphCoP( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame, int step ){

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
			ViewXYPlotClippedDoubles( view, &RealMarkerTime[0], &CenterOfPressure[ati][0][i], start_frame, stop_frame, step, sizeof( *RealMarkerTime ), sizeof( *CenterOfPressure[ati] ), MISSING_DOUBLE );
		}
	}
}

// Phase plots of Manipulandum position data.
void GripMMIDesktop::PlotManipulandumPosition( double start_instant, double stop_instant, int start_frame, int stop_frame, int step ){

	::View view;

	for ( int i = 0; i < PHASEPLOTS - 1; i++ ) {

		DisplayActivate( phase_display[i] );
		Erase( phase_display[i] );
		view = phase_view[i];
		ViewSetXLimits( view, lowerPositionLimitSpecific[pair[i].abscissa], upperPositionLimitSpecific[pair[i].abscissa] );
		ViewSetYLimits( view, lowerPositionLimitSpecific[pair[i].ordinate], upperPositionLimitSpecific[pair[i].ordinate] );
		ViewMakeSquare( view );
		ViewSelectColor( view, i );
		// ViewBox( view );
		if ( stop_frame > start_frame ) ViewXYPlotAvailableDoubles( view, &ManipulandumPosition[0][pair[i].abscissa], &ManipulandumPosition[0][pair[i].ordinate], start_frame, stop_frame, step, sizeof( *ManipulandumPosition ), sizeof( *ManipulandumPosition ), MISSING_DOUBLE );
		OglSwap( phase_display[i] );
	}
}

// Phase plots of center-of-pressure data.
void GripMMIDesktop::PlotCoP( double start_instant, double stop_instant, int start_frame, int stop_frame, int step ){

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
		ViewScatterPlotAvailableDoubles( view, SYMBOL_FILLED_SQUARE, &CenterOfPressure[RIGHT_ATI][0][Z], &CenterOfPressure[RIGHT_ATI][0][Y], start_frame, stop_frame, step, sizeof( *CenterOfPressure[RIGHT_ATI] ), sizeof( *CenterOfPressure[RIGHT_ATI] ), MISSING_FLOAT );
		ViewColor( view, atiColorMap[LEFT_ATI] );
		ViewScatterPlotAvailableDoubles( view, SYMBOL_FILLED_SQUARE, &CenterOfPressure[LEFT_ATI][0][Z], &CenterOfPressure[LEFT_ATI][0][Y], start_frame, stop_frame, step, sizeof( *CenterOfPressure[LEFT_ATI] ), sizeof( *CenterOfPressure[0] ), MISSING_FLOAT );
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
