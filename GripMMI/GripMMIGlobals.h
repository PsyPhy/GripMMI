#pragma once

///
/// Module:	GripMMI
/// 
///	Author:					J. McIntyre, PsyPhy Consulting
/// Initial release:		18 December 2014
/// Modification History:	see https://github.com/frenchjam/GripGroundMonitorClient
///
/// Copyright (c) 2014, 2015 PsyPhy Consulting
///

/// Definition of preprocessor constants and global variables for GripMMI.

/// <summary>
/// Buffers to hold the GRIP data.
/// </summary>
#define MAX_FRAMES (12*60*60*20) // Max number of frames (data slices)
#define CODA_MARKERS 20
#define	CODA_UNITS	2
#define MANIPULANDUM_FIRST_MARKER 0
#define MANIPULANDUM_LAST_MARKER  7
extern Vector3 ManipulandumRotations[MAX_FRAMES];
extern Vector3 ManipulandumPosition[MAX_FRAMES];
extern Vector3 Acceleration[MAX_FRAMES];
extern float GripForce[MAX_FRAMES];
extern Vector3 LoadForce[MAX_FRAMES];
extern float NormalForce[N_FORCE_TRANSDUCERS][MAX_FRAMES];
extern double LoadForceMagnitude[MAX_FRAMES];
extern Vector3 CenterOfPressure[N_FORCE_TRANSDUCERS][MAX_FRAMES];
extern float RealMarkerTime[MAX_FRAMES];
extern float CompressedMarkerTime[MAX_FRAMES];
extern float RealAnalogTime[MAX_FRAMES];
extern float CompressedAnalogTime[MAX_FRAMES];
extern char  MarkerVisibility[MAX_FRAMES][CODA_MARKERS];
extern char  ManipulandumVisibility[MAX_FRAMES];
extern char markerVisibilityString[CODA_UNITS][32];
extern unsigned int nFrames;
/// <summary>
/// Data display.
/// </summary>
#define PHASEPLOTS 3
#define STRIPCHARTS	6
#define SPAN_VALUES	6
extern int windowSpan[SPAN_VALUES]; // Number of samples to plot for each position of the span selector.
/// <summary>
/// Strings used to construct the path to various files.
/// </summary>
#define MAX_PATHLENGTH	1024
extern char packetBufferPathRoot[MAX_PATHLENGTH];
extern char scriptDirectory[MAX_PATHLENGTH];
extern char pictureFilenamePrefix[MAX_PATHLENGTH];
/// <summary>
/// Buffers to hold the contents of the scripts.
/// </summary>
#define MAX_TOKENS				32
#define MAX_STEPS				4096
#define MAX_MENU_ITEMS			256
#define MAX_MENU_ITEM_LENGTH	1024
extern char picture[MAX_STEPS][256];
extern char message[MAX_STEPS][132];
extern char *type[MAX_STEPS];
extern bool comment[MAX_STEPS];
/// <summary>
/// Constants to translate enums to strings.
/// </summary>
extern char *soundBar[16];
extern char *massDecoder[4];
/// <summary>
/// A helper object for performing vector ops and DEX data ops.
/// </summary>
extern DexAnalogMixin	dex;
