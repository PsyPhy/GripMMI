///
/// Module:	GripMMI
/// 
///	Author:					J. McIntyre, PsyPhy Consulting
/// Initial release:		18 December 2014
/// Modification History:	see https://github.com/frenchjam/GripGroundMonitorClient
///
/// Copyright (c) 2014, 2015 PsyPhy Consulting
///

/// GripMMI global static variables.

// My original code in VC 6 used static arrays with known dimensions, and an number of (char *) objects.
// VC 2010 doesn't allow 'mixed' types in Forms objects. So here I define a number of global variables to 
//  replace the arrays that I was using previously. It's ugly, but I don't have time to convert it all.

#include "StdAfx.h"
#include "..\Grip\DexAnalogMixin.h"
#include "..\Grip\GripPackets.h"
#include "GripMMIGlobals.h"

// Time span in seconds for each position of the span selector.
double windowSpanSeconds[SPAN_VALUES] = { 43200.0, 14400.0, 3600.0, 1800.0, 600.0, 300.0, 60.0, 30.0 };

// Character strings to indicate the state of the tone generator output.
// Lowest bit on is a mute switch, so odd elements are empty while each
// even element has a bar who's left-right position represents a frequency.
char *soundBar[16] = {
	"|.......",
	"........",
	".|......",
	"........",
	"..|.....",
	"........",
	"...|....",
	"........",
	"....|...",
	"........",
	".....|..",
	"........",
	"......|.",
	"........",
	".......|",
	"........" 
};

// Decode the 2 bits of the mass detectors in the cradles.
// 00 = empty, 10 = 400gm, 01 = 600gm, 11 = 800gm
// Note discrepancy in DEX-ICD-00383-QS Iss E Rev 1 where in one place it is stated that bit 0 is LSB
//  and that 00 = no mass, 01 = 400 gm, 10 = 600 gm, 11 = 800 gm. Bert confirms that the bit order 
//  is actually the EPM order (bit 0 is MSB). Therefore 01 = 600 gm and 10 = 400 gm.
char *massDecoder[4] = {".", "M", "S", "L" };

// Data buffers
Vector3 ManipulandumRotations[MAX_FRAMES];
Vector3 ManipulandumPosition[MAX_FRAMES];
Vector3 Acceleration[MAX_FRAMES];
double GripForce[MAX_FRAMES];
Vector3 LoadForce[MAX_FRAMES];
double NormalForce[N_FORCE_TRANSDUCERS][MAX_FRAMES];
double LoadForceMagnitude[MAX_FRAMES];
Vector3 CenterOfPressure[N_FORCE_TRANSDUCERS][MAX_FRAMES];
double RealMarkerTime[MAX_FRAMES];
double CompressedMarkerTime[MAX_FRAMES];
double RealAnalogTime[MAX_FRAMES];
double CompressedAnalogTime[MAX_FRAMES];
double  MarkerVisibility[MAX_FRAMES][CODA_MARKERS];
double  ManipulandumVisibility[MAX_FRAMES];
double  FrameVisibility[MAX_FRAMES];
double  WristVisibility[MAX_FRAMES];
double  PacketReceived[MAX_FRAMES];
char markerVisibilityString[CODA_UNITS][32];
unsigned int nFrames = 0;

// This value is used to adjust timestamps to align packet times
//  to a specific timebase. For instance, EPM uses GPS time, which 
//  ignores leap seconds. To get true UTC, this should be set to the
//  number of leap seconds since midnight, Jan. 6, 1980. 
int TimebaseOffset = -16;

// A helper object
DexAnalogMixin	dex;