
// My original code in VC 6 used static arrays with known dimensions, and an number of (char *) objects.
// VC 2010 doesn't allow 'mixed' types in Forms objects. So here I define a number of global variables to 
//  replace the arrays that I was using previously. It's ugly, but I don't have time to convert it all.

#include "StdAfx.h"
#include "..\Grip\DexAnalogMixin.h"
#include "..\Grip\GripPackets.h"
#include "GripMMIGlobals.h"

// Number of samples to plot for each position of the span selector.
int windowSpan[SPAN_VALUES]  = { 20 * 60 * 60, 20 * 60 * 30, 20 * 60 * 10, 20 * 60 * 5, 20 * 60, 20 * 30 };

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
float GripForce[MAX_FRAMES];
Vector3 LoadForce[MAX_FRAMES];
float NormalForce[N_FORCE_TRANSDUCERS][MAX_FRAMES];
double LoadForceMagnitude[MAX_FRAMES];
Vector3 CenterOfPressure[N_FORCE_TRANSDUCERS][MAX_FRAMES];
float RealMarkerTime[MAX_FRAMES];
float CompressedMarkerTime[MAX_FRAMES];
float RealAnalogTime[MAX_FRAMES];
float CompressedAnalogTime[MAX_FRAMES];
char  MarkerVisibility[MAX_FRAMES][CODA_MARKERS];
char  ManipulandumVisibility[MAX_FRAMES];
char markerVisibilityString[CODA_UNITS][32];
unsigned int nFrames = 0;

// A helper object
DexAnalogMixin	dex;