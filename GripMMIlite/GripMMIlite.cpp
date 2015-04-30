///
/// Module:	GripMMIlite (GripMMI)
/// 
///	Author:					J. McIntyre, PsyPhy Consulting
/// Initial release:		18 December 2014
/// Modification History:	see https://github.com/PsyPhy/GripMMI
///
/// Copyright (c) 2014, 2015 PsyPhy Consulting

//
// This program provides a simplified monitoring of Grip realtime data downlink.
// A subset of the data is written to stdout, to be displayed on the console.
// The program uses the same mechanism as the main GripMMI GUI to retieve data from
//  the EPM server. It can therefore be used to debug the connection to the EPM.

// The program takes as input packets stored in two cache files: XXX.rt.gpk and XXX.hk.gpk.
// The root paths to these files (replacing the XXX) can be specified at the first 
//  parameter on the command line. The default values is ".\\GripPackets".
// The program continually reads all the packets from these two cache files and if there
//  are new packets in either file with respect to the previous iteraion, it outputs
//  a subset of the data to stdout, i.e. to the console for display.

#include "stdafx.h"
#include "..\Grip\GripPackets.h"
#include "..\Useful\fMessageBox.h"
#include "..\Useful\fOutputDebugString.h"

EPMTelemetryPacket hkPacket;
EPMTelemetryPacket rtPacket;
EPMTelemetryHeaderInfo epmHeader;
GripHealthAndStatusInfo		hkInfo;
GripRealtimeDataInfo		rtInfo;

// Default path for packet storage is the current directory.
const char *packetCacheFilenameRoot = ".\\GripPackets";

// Buffers to hold the path to the packet caches.
char rtPacketCacheFilePath[1024];
char hkPacketCacheFilePath[1024];

// Max times to try to open the cache file before asking user to continue or not.
#define MAX_OPEN_CACHE_RETRIES	5
// Pause time in milliseconds between file open retries.
#define RETRY_PAUSE	2000		
// Error code to return if the cache file cannot be opened.
#define ERROR_CACHE_NOT_FOUND	-1000

// Character strings to indicate the state of the tone generator output.
// Lowest bit on is a mute switch, so odd elements are empty while each
// even element has a bar who's position between the brackets represents
// a frequency.
char *soundBar[16] = {
	"[|       ]",
	"[        ]",
	"[ |      ]",
	"[        ]",
	"[  |     ]",
	"[        ]",
	"[   |    ]",
	"[        ]",
	"[    |   ]",
	"[        ]",
	"[     |  ]",
	"[        ]",
	"[      | ]",
	"[        ]",
	"[       |]",
	"[        ]" };

// Decode the 2 bits of the mass detectors in the cradles.
// 00 = empty, 01 = 400gm, 10 = 600gm, 11 = 800gm
char *massDecoder = ".SML";

BOOL readHK ( char *filename, EPMTelemetryPacket *packet ) {

	static int count = 0;

	int  fid;
	int packets_read = 0;
	int bytes_read;
	int return_code;
	static unsigned short previousTMCounter = 0;
	unsigned short bit = 0;
	int retry_count;
	int mb_answer;

	// Attempt to open the packet cache to read the accumulated packets.
	// If it is not immediately available, try for a few seconds then query the user.
	// The user can choose to continue to wait or cancel program execution.
	do {
		for ( retry_count = 0; retry_count  < MAX_OPEN_CACHE_RETRIES; retry_count ++ ) {
			// If open succeeds, it will return zero. So if zero return, break from retry loop.
			return_code = _sopen_s( &fid, filename, _O_RDONLY | _O_BINARY, _SH_DENYNO, _S_IWRITE | _S_IREAD  );
			if ( return_code == 0 ) break;
			// Wait a second before trying again.
			Sleep( RETRY_PAUSE );
		}
		// If return_code is zero, file is open, so break out of loop and continue.
		if ( return_code == 0 ) break;
		// If return_code is non-zero, we are here because the retry count has been reached without opening the file.
		// Ask the user if they want to keep on trying or abort.
		else {
			mb_answer = fMessageBox( MB_RETRYCANCEL, "GripMMIlite", "Error opening %s for binary read.\nContinue trying?", filename );
			if ( mb_answer == IDCANCEL ) exit( ERROR_CACHE_NOT_FOUND ); // User chose to abort.
		}
	} while ( true ); // Keep trying until success or until user cancels.

	// Read in all of the data packets in the file.
	packets_read = 0;
	while ( hkPacketLengthInBytes == (bytes_read = _read( fid, packet, hkPacketLengthInBytes )) ) {
		packets_read++;
		if ( bytes_read < 0 ) {
			fMessageBox( MB_OK, "GripMMIlite", "Error reading from %s.", filename );
			exit( -1 );
		}
		// Check that it is a valid GRIP packet. It would be strange if it was not.
		ExtractEPMTelemetryHeaderInfo( &epmHeader, packet );
		if ( epmHeader.epmSyncMarker != EPM_TELEMETRY_SYNC_VALUE || epmHeader.TMIdentifier != GRIP_HK_ID ) {
			fMessageBox( MB_OK, "GripMMIlite", "Unrecognized packet from %s.", filename );
			exit( -1 );
		}
	}
	// Finished reading. Close the file and check for errors.
	return_code = _close( fid );
	if ( return_code ) {
		fMessageBox( MB_OK, "GripMMIlite", "Error closing %s after binary read.\nError code: %s", filename, return_code );
		exit( return_code );
	}
	// Check if there were new packets since the last time we read the cache.
	// Return TRUE if yes, FALSE if no.
	if ( previousTMCounter != epmHeader.TMCounter ) {
		previousTMCounter = epmHeader.TMCounter;
		return( TRUE );
	}
	else return ( FALSE );
}

BOOL readRT ( char *filename, EPMTelemetryPacket *packet ) {

	static int count = 0;

	int  fid;
	int packets_read = 0;
	int bytes_read;
	int return_code;
	int retry_count;
	int mb_answer;
	static unsigned short previousTMCounter = 0;
	unsigned short bit = 0;

	// Attempt to open the packet cache to read the accumulated packets.
	// If it is not immediately available, try for a few seconds then query the user.
	// The user can choose to continue to wait or cancel program execution.
	do {
		for ( retry_count = 0; retry_count  < MAX_OPEN_CACHE_RETRIES; retry_count ++ ) {
			// If open succeeds, it will return zero. So if zero return, break from retry loop.
			return_code = _sopen_s( &fid, filename, _O_RDONLY | _O_BINARY, _SH_DENYNO, _S_IWRITE | _S_IREAD  );
			if ( return_code == 0 ) break;
			// Wait a second before trying again.
			Sleep( RETRY_PAUSE );
		}
		// If return_code is zero, file is open, so break out of loop and continue.
		if ( return_code == 0 ) break;
		// If return_code is non-zero, we are here because the retry count has been reached without opening the file.
		// Ask the user if they want to keep on trying or abort.
		else {
			mb_answer = fMessageBox( MB_RETRYCANCEL, "GripMMIlite", "Error opening %s for binary read.\nContinue trying?", filename );
			if ( mb_answer == IDCANCEL ) exit( ERROR_CACHE_NOT_FOUND ); // User chose to abort.
		}
	} while ( true ); // Keep trying until success or until user cancels.

	// Read in all of the data packets in the file.
	packets_read = 0;
	while ( rtPacketLengthInBytes == (bytes_read = _read( fid, packet, rtPacketLengthInBytes )) ) {
		packets_read++;
		if ( bytes_read < 0 ) {
			fMessageBox( MB_OK, "GripMMIlite", "Error reading from %s.", filename );
			exit( -1 );
		}
		ExtractEPMTelemetryHeaderInfo( &epmHeader, packet );
		if ( epmHeader.epmSyncMarker != EPM_TELEMETRY_SYNC_VALUE || epmHeader.TMIdentifier != GRIP_RT_ID ) {
			fMessageBox( MB_OK, "GripMMIlite", "Unrecognized packet from %s.", filename );
			exit( -1 );
		}
	}
	return_code = _close( fid );
	if ( return_code ) {
		fMessageBox( MB_OK, "GripMMIlite", "Error closing %s after binary read.\nError code: %s", filename, return_code );
		exit( return_code );
	}
	if ( previousTMCounter != epmHeader.TMCounter ) {
		previousTMCounter = epmHeader.TMCounter;
		return( TRUE );
	}
	else return ( FALSE );

}


int main(int argc, char *argv[])
{
	BOOL new_hk;
	BOOL new_rt;

	int i;

	if ( argc < 2 ) printf( "Using default root path for cache files: %s\n", packetCacheFilenameRoot );
	else {
		packetCacheFilenameRoot = argv[1];
		printf( "Using command-line root path for cache files: %s\n", packetCacheFilenameRoot );
	}

	// Construct the paths to the cache files where we expect to read the incoming packets.
	CreateGripPacketCacheFilename( hkPacketCacheFilePath, sizeof( hkPacketCacheFilePath ), GRIP_HK_BULK_PACKET, packetCacheFilenameRoot );
	CreateGripPacketCacheFilename( rtPacketCacheFilePath, sizeof( rtPacketCacheFilePath ), GRIP_RT_SCIENCE_PACKET, packetCacheFilenameRoot );

	while ( 1 ) {

		// Read the cache files and leave the last packet in each packet buffer.
		// Each routine returns TRUE if new packets have been added, FALSE otherwise.
		new_hk = readHK( hkPacketCacheFilePath, &hkPacket );
		new_rt = readRT( rtPacketCacheFilePath, &rtPacket );

		// Output a new line to the display if one or the packet caches has new info.
		if ( new_hk || new_rt ) {

			// Show the pertinent information from the last frame read.
			ExtractGripHealthAndStatusInfo( &hkInfo, &hkPacket );
			// Where are we in the script.
//			fprintf( stdout, "  User: %3d Protocol: %3d Task: %3d Step: %3d", hkInfo.user, hkInfo.protocol, hkInfo.task, hkInfo.step );
#if 0
			// State of the horizontal and vertical target LEDs.
			fprintf( stdout, " H: " );
			for ( i = 0, bit = 0x01; i < 10; i++, bit = bit << 1 ) {
				if ( bit & hkInfo.horizontalTargetFeedback ) fprintf( stdout, "O" );
				else fprintf( stdout, "." );
			}
			fprintf( stdout, "  V: " );
			for ( i = 0, bit = 0x01; i < 13; i++, bit = bit << 1 ) {
				if ( bit & hkInfo.verticalTargetFeedback ) fprintf( stdout, "O" );
				else fprintf( stdout, "." );
			}
			// State of the tone generator.
			fprintf( stdout, "  S: %s", soundBar[hkInfo.toneFeedback] );
			fprintf( stdout, "  M: %c%c%c", 
				massDecoder[ hkInfo.cradleDetectors & 0x03 ],
				massDecoder[ (hkInfo.cradleDetectors >> 2) & 0x03 ],
				massDecoder[ (hkInfo.cradleDetectors >> 4) & 0x03 ] );
			// Indicate if coda and/or camera is acquiring.
			fprintf( stdout, "  %s  %s", 
				( hkInfo.motionTrackerStatusEnum == 2 ? "acquiring" : "    -    " ),
				( hkInfo.crewCameraStatusEnum == 2 ? "smile!" : "  -   " )
				);
#endif 
			// Show some of the realtime science data.
			ExtractGripRealtimeDataInfo( &rtInfo, &rtPacket );
			// Show the acquisition count and the GRIP realtime packet count.
			fprintf( stdout, "%4d %8d", rtInfo.acquisitionID, rtInfo.rtPacketCount );
			fprintf( stdout, " %lf", rtInfo.packetTimestamp );
			// Show for each slice in the packet the tick for the coda data
			//  and whether or not the manipulandum is visible.
			for ( i = 0; i < RT_SLICES_PER_PACKET; i++ ) {
				fprintf( stdout, " %6d%", rtInfo.dataSlice[i].poseTick );
//				fprintf( stdout, " %8.3f", rtInfo.dataSlice[i].bestGuessPoseUTC );
//				fprintf( stdout, " %6d%c", rtInfo.dataSlice[i].analogTick, ( rtInfo.dataSlice[i].manipulandumVisibility ? '+' : '-' ) );
//				fprintf( stdout, " %6.2f", rtInfo.dataSlice[i].acceleration[Z] );
			}
			fprintf( stdout, "\n" );
			fprintf( stdout, "%4d %8d", rtInfo.acquisitionID, rtInfo.rtPacketCount );
			fprintf( stdout, " %lf", rtInfo.packetTimestamp );
			for ( i = 0; i < RT_SLICES_PER_PACKET; i++ ) {
				fprintf( stdout, " %6d", rtInfo.dataSlice[i].analogTick );
//				fprintf( stdout, " %8.3f", rtInfo.dataSlice[i].bestGuessPoseUTC );
//				fprintf( stdout, " %6d%c", rtInfo.dataSlice[i].analogTick, ( rtInfo.dataSlice[i].manipulandumVisibility ? '+' : '-' ) );
//				fprintf( stdout, " %6.2f", rtInfo.dataSlice[i].acceleration[Z] );
			}
			fprintf( stdout, "\n\n" );

		}

		// Break out of loop on any keypress.
		if ( _kbhit() ) {
			// Remove the keystroke from the buffer.
			_getch();
			// Break out of the continuous loop.
			break;
		}
		Sleep( 20 );
	}
	return 0;
}

