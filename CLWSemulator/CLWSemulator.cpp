///
/// Module:	CLWSemulator (GripMMI)
/// 
///	Author:					J. McIntyre, PsyPhy Consulting
/// Initial release:		18 December 2014
/// Modification History:	see https://github.com/PsyPhy/GripMMI
///
/// Copyright (c) 2014, 2015 PsyPhy Consulting
///

// This program provides the means to test the Grip MMI by emulating the CLWS data server.
// It provides an TCP/IP server that outputs packets similar to those sent by EPM during
//  execution of experiments on the Grip experiment module.

// This emulator works in one of two modes. It can construct artificial packets containing 
//  a subset of the expected data that is representative of what one expects to see during
//  a Grip experiment. ALternatively, it can play back packets that have been previously 
//  stored from a real session of Grip.

#include "stdafx.h"
#include "..\Useful\fMessageBox.h"
#include "..\Useful\fOutputDebugString.h"
#include "..\Grip\DexAnalogMixin.h"
#include "..\Grip\GripPackets.h"
#include "..\GripMMI\GripMMIGlobals.h"
#include "..\GripMMIVersionControl\GripMMIVersionControl.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

PCSTR EPMport = EPM_DEFAULT_PORT;

// Path to a file containing a mixture of different packet types.
// These packets were stored during a real (albeit abbreviated) Grip sesion.
// This is the default path to the file, but it can be changed on the command line.
const char *DefaultPacketSourceFile = ".\\GripPacketsForSimulator.gpk";

// The bool _debug controls whether certain debugging information is sent to the console.
// It is true by default when compiled in debug mode and false when compiled in release mode.
#ifdef _DEBUG
	bool _debug = true;
#else
	bool _debug = false;
#endif

// The bool verbose determines how much information we send to the console during normal operation.
bool verbose = false;

void setPacketTime( EPMTelemetryHeaderInfo *header ) {

	// Set the time of an EPM telemetry packet.

	// Time structure to get the local time for the EPM packet coarse and fine time values.
	// I am using the 32 bit version because the EPM coarse time is 32 bits.
	struct __timeb32 epmtime;
	_ftime32_s( &epmtime );

	// NB EPM uses GPS time (second since midnight Jan 5-6 1980), while 
	// _ftime_s() uses seconds since midnight Jan. 1 1970 UTC.
	// UTC takes into account leap seconds, GPS does not.
	header->coarseTime = epmtime.time 
		- 315964800 // Offset in seconds between Unix 0 and GPS 0
		+ 16;		// Offset taking into account leap seconds, as of 1 Jan 2015.

	// Also, EPM somehow gets time in 10ths of milliseconds and puts that in the header. 
	// We don't expect to get two packets in a span of less than a millisecond, so I don't worry about it.
	header->fineTime = epmtime.millitm * 10;

}

// This is the routine that sends out packets that were pre-recorded.
// Takes as its input the socket for outputing packets, the path to the file
// containing the recorded packets.
int sendRecordedPackets ( SOCKET socket, const char *PacketSourceFile ) {

	// Count the total numbe of packets sent on the socket.
	static int packetCount = 0;

	int	fid;
	int return_code;
	int bytes_read;
    int iSendResult;
			
	EPMTelemetryHeaderInfo epmPacketHeaderInfo;
	EPMTelemetryPacket recordedPacket;

	while ( 1 ) {

		printf( "Sending out recorded packets:\n\n  %s\n\n", PacketSourceFile );

		// Open the file where the packets are stored.
		return_code = _sopen_s( &fid, PacketSourceFile, _O_RDONLY | _O_BINARY, _SH_DENYNO, _S_IWRITE | _S_IREAD  );
		if ( return_code ) {
			fMessageBox( MB_OK, "CLWSemulator", "Error opening %s for binary read.", PacketSourceFile );
			exit( -1 );
		}
		// Read the first one, and exit with an error if there is not at least one packet.
		bytes_read = _read( fid, recordedPacket.buffer, sizeof( recordedPacket.buffer ) );
		if ( bytes_read < 0 ) {
			fMessageBox( MB_OK, "CLWSemulator", "Error reading from %s.", PacketSourceFile );
			exit( -1 );
		}
		// Extract the EPM header info into a usable form from the packet that is stored in ESA-required byte order.
		// Here we use it to initialize the record of the time of the previous packet, which is used
		// later to compute the time between recorded packets and to sleep accordingly.
		ExtractEPMTelemetryHeaderInfo( &epmPacketHeaderInfo, &recordedPacket );
		unsigned int previous_coarse_time = epmPacketHeaderInfo.coarseTime;
		unsigned int previous_fine_time = epmPacketHeaderInfo.fineTime;
		
		// Loop to read all of the packets in the file.
		do {
			// Extract the EPM header info into a usable form from the packet that is stored in ESA-required byte order.
			ExtractEPMTelemetryHeaderInfo( &epmPacketHeaderInfo, &recordedPacket );
			// Check what type of packet it is.
			if ( epmPacketHeaderInfo.epmSyncMarker != EPM_TELEMETRY_SYNC_VALUE ) {
				// If it's not an EPM packet we don't send it out.
				if ( verbose ) printf( "Bytes: %4d (non EPM).\n", bytes_read ); 
			}
			else {
				if ( verbose ) printf( "Bytes: %4d EPM.\n", bytes_read );
				// If it is not a GRIP packet, just show that we are progressing through the packets.
				if ( epmPacketHeaderInfo.subsystemID != GRIP_SUBSYSTEM_ID ) printf( "." );
				// If it is a GRIP packet, modify the pre-recorded packet too make it look like it was generated just now.
				else {
					// Compute the number of milliseconds between this packet and the previous one.
					// This will be used further down to sleep the appropriate time to space out the packets.
					int delta_milliseconds = ((int) epmPacketHeaderInfo.fineTime - (int) previous_fine_time ) / 10;
					int delta_time = (epmPacketHeaderInfo.coarseTime - previous_coarse_time) * 1000 + delta_milliseconds;
					// Store the current time as a reference for the next cycle.
					previous_coarse_time = epmPacketHeaderInfo.coarseTime;
					previous_fine_time = epmPacketHeaderInfo.fineTime;
					// Set the timestamp of the packet to be output to the current time.
					setPacketTime( &epmPacketHeaderInfo );
					// Set the packet counter based on a local count.
					epmPacketHeaderInfo.TMCounter = packetCount++;
					// Put the new header info back into the packet.
					InsertEPMTelemetryHeaderInfo( &recordedPacket, &epmPacketHeaderInfo );
					// Send it out on the socket.
					iSendResult = send( socket, recordedPacket.buffer, EPM_BUFFER_LENGTH - 1, 0 );
					// If we get a socket error it is probably because the client has closed the connection.
					// So we break out of the loop.
					if (iSendResult == SOCKET_ERROR) {
						printf( "Recorded packet send() failed with error: %3d\n", WSAGetLastError());
						return( packetCount );
					}
					// Sleep based on the difference in time between the previousrecorded packet and this one.
					// If there has been a long real delay, limit it to 30 seconds.
					delta_time %= 30000;
					printf( "G%d", delta_time );
					Sleep( delta_time );

				}
			}
		// Loop until there are no more bytes to read.
		} while ( bytes_read = _read( fid, recordedPacket.buffer, sizeof( recordedPacket.buffer ) ) );

		// Try to gracefully close the file.
		return_code = _close( fid );
		if ( return_code ) {
			fMessageBox( MB_OK, "CLWSemulator", "Error closing %s after binary read.\nError code: %s", PacketSourceFile, return_code );
			exit( return_code );
		}

		// Sleep to simulate a pause in the experiment execution, then start over again.
		printf( "Playback completed. Will restart in 10 seconds.\n" );
		Sleep( 10000 );

	}

}

// This is the routine that sends out packets that are constructed here to simulate data.
// Takes as its only input the socket for outputing packets.
int sendConstructedPackets ( SOCKET socket ) {

	EPMTelemetryPacket hkPacket, rtPacket;
	EPMTelemetryHeaderInfo hkHeaderInfo, rtHeaderInfo;
	GripHealthAndStatusInfo hkInfo;
	GripRealtimeDataInfo rtInfo;
	GripRealtimeDataInfo reverseInfo;

	// Flag values for visible targets, sound generator and cradles.
	unsigned short vertical_targets = 0;
	unsigned short horizontal_targets = 0;
	unsigned char sound_bits = 0;
	unsigned char cradle_bits = 0;
	int camera_status = 2;
	int acquire_status = 2;

	int packet_count = 0;
	int rt_packet_count = 0;
	// A flag used to send one housekeeping packet for every two realtime data packets.
	int send_hk = 1;
	// A counter to generate simulated occlusions of the manipulandum.
	int dropout_count = 0;

    int iSendResult;

	// Prepare the packets by copying constants into local structure.
	memcpy( &hkHeaderInfo, &hkHeader, sizeof( hkHeaderInfo ) );
	memcpy( &rtHeaderInfo, &rtHeader, sizeof( rtHeaderInfo ) );

	// Send packets in short periods that we will call epochs.
	// Breaks between epochs will simulate pauses in GRIP execution
	//  on board or LOS periods.
	int epoch = 0;

	// Send packets until the peer shuts down the connection
	while ( 1 ) {

		// RT packets get sent out by GRIP twice per second.
		// This is a trick to avoid drift in the rate.
		// First, make sure that we sleep enough so as not to repeat.
		Sleep( 50 );	
		// Then compute the number of milliseconds to sleep to get back to a 500 ms boundary.
		struct __timeb32 utctime;
		_ftime32_s( &utctime );
		Sleep( (1000 - utctime.millitm ) % 500 );

		// Insert the current packet count and time into the packet.
		rtHeaderInfo.TMCounter = packet_count++;
		setPacketTime( &rtHeaderInfo );
		InsertEPMTelemetryHeaderInfo( &rtPacket, &rtHeaderInfo );

		// Fabricate data values for the real-time packets.
		rtInfo.packetTimestamp = EPMtoSeconds( &rtHeaderInfo );
		fOutputDebugString( "Timestamp: %.3f\n",  rtInfo.packetTimestamp );
		rtInfo.acquisitionID = 0;
		rtInfo.rtPacketCount = rt_packet_count++;

		// There are RT_SLICES_PER_PACKET slices of data in each packet.
		for ( int slice = 0; slice < RT_SLICES_PER_PACKET; slice++ ) {

			rtInfo.dataSlice[slice].poseTick = rt_packet_count * RT_SLICES_PER_PACKET;
			rtInfo.dataSlice[slice].analogTick = rt_packet_count * RT_SLICES_PER_PACKET;

			// Initialize to zero position, acceleration and forces and to null orientation.
			for ( int i = X; i <= Z; i++ ) {
				rtInfo.dataSlice[slice].position[i] = 0.0f;
				rtInfo.dataSlice[slice].acceleration[i] = 0.0f;
				rtInfo.dataSlice[slice].ft[0].force[i] = 0.0f;
				rtInfo.dataSlice[slice].ft[1].force[i] = 0.0f;
				rtInfo.dataSlice[slice].ft[0].torque[i] = 0.0f;
				rtInfo.dataSlice[slice].ft[1].torque[i] = 0.0f;
				rtInfo.dataSlice[slice].quaternion[i] = 0.0f;
			}
			// Finish initializing the null orientation.
			rtInfo.dataSlice[slice].quaternion[M] = 1.0f;
			// Now generate various patterns of movement and forces.
			// Most are computed from a 1 Hz sinusoid.
			double t = rtInfo.packetTimestamp + slice * RT_DEFAULT_SECONDS_PER_SLICE;
			double s = sin( t * Pi * 2.0 );
			double c = cos( t * Pi * 2.0 );
			// The pattern changes each epoque, but recycles every 6 epochs.
			// Note that position, orientation, acceleration and force data are not 
			//  necessarily coherent with each other. We are just trying to generate
			//  some data to be plotted by GripMMI.
			switch ( epoch % 6 ) {
			case 0: // Oscillating left-right movement.
				rtInfo.dataSlice[slice].position[X] = 300.0 + 300.0 * c;
				rtInfo.dataSlice[slice].acceleration[X] = - 300.0 * c * RT_DEFAULT_SECONDS_PER_SLICE * RT_DEFAULT_SECONDS_PER_SLICE;
				rtInfo.dataSlice[slice].ft[0].force[X] = - 14.0 + 8.5 * s;
				rtInfo.dataSlice[slice].ft[1].force[X] =  - rtInfo.dataSlice[slice].ft[0].force[X];
				// Wrist and frame visible.
				rtInfo.dataSlice[slice].markerVisibility[0] = 0x000ff;
				rtInfo.dataSlice[slice].markerVisibility[1] = 0xf0fff;
				break;
			case 1: // Oscillating up-down movement.
				rtInfo.dataSlice[slice].position[Y] = 300.0 + 300.0 * c;
				rtInfo.dataSlice[slice].acceleration[Y] = - 300.0 * c * RT_DEFAULT_SECONDS_PER_SLICE * RT_DEFAULT_SECONDS_PER_SLICE;
				rtInfo.dataSlice[slice].ft[0].force[Y] = 2.0 * s;
				rtInfo.dataSlice[slice].ft[1].force[Y] = 1.8 * s;
				// Wrist visible, frame occluded.
				rtInfo.dataSlice[slice].markerVisibility[0] = 0x000ff;
				rtInfo.dataSlice[slice].markerVisibility[1] = 0x0f0ff;
				break;
			case 2: // Oscillating in-out movement.
				rtInfo.dataSlice[slice].position[Z] = - 300.0 + 200.0 * c;
				rtInfo.dataSlice[slice].acceleration[Z] = - 200.0 * c * RT_DEFAULT_SECONDS_PER_SLICE * RT_DEFAULT_SECONDS_PER_SLICE;
				rtInfo.dataSlice[slice].ft[0].force[Z] = 3.0 * s;
				rtInfo.dataSlice[slice].ft[1].force[Z] = 3.2 * s;
				// Frame visible, wrist occluded.
				rtInfo.dataSlice[slice].markerVisibility[1] = 0x000ff;
				rtInfo.dataSlice[slice].markerVisibility[0] = 0x00fff;
				break;
			case 3: 
				// Pitch rotations.
				rtInfo.dataSlice[slice].quaternion[X] = s / 2.0;
				rtInfo.dataSlice[slice].quaternion[M] = c / 2.0;
				// Horizontal sliding CoP.
				rtInfo.dataSlice[slice].ft[0].force[X] = - 14.0 + 8.5 * s;
				rtInfo.dataSlice[slice].ft[1].force[X] =  - rtInfo.dataSlice[slice].ft[0].force[X];
				rtInfo.dataSlice[slice].ft[0].torque[Y] = rtInfo.dataSlice[slice].ft[0].force[X] * 0.01 * s;
				rtInfo.dataSlice[slice].ft[1].torque[Y] = rtInfo.dataSlice[slice].ft[1].force[X] * 0.011 * s;
				// Wrist and frame visible.
				rtInfo.dataSlice[slice].markerVisibility[1] = 0xfffff;
				rtInfo.dataSlice[slice].markerVisibility[0] = 0xf0f0f;
				break;
			case 4: 
				// Yaw rotations.
				rtInfo.dataSlice[slice].quaternion[Y] = s / 2.0;
				rtInfo.dataSlice[slice].quaternion[M] = c / 2.0;
				// Vertical sliding CoP.
				rtInfo.dataSlice[slice].ft[0].force[X] = - 14.0 + 8.5 * s;
				rtInfo.dataSlice[slice].ft[1].force[X] =  - rtInfo.dataSlice[slice].ft[0].force[X];
				rtInfo.dataSlice[slice].ft[0].torque[Z] = rtInfo.dataSlice[slice].ft[0].force[X] * 0.01 * s;
				rtInfo.dataSlice[slice].ft[1].torque[Z] = rtInfo.dataSlice[slice].ft[1].force[X] * 0.011 * s;
				// Wrist and frame visible.
				rtInfo.dataSlice[slice].markerVisibility[1] = 0xfffff;
				rtInfo.dataSlice[slice].markerVisibility[0] = 0x0f0f0;
				break;
			case 5:
				// Roll rotations.
				rtInfo.dataSlice[slice].quaternion[Z] = s / 2.0;
				rtInfo.dataSlice[slice].quaternion[M] = c / 2.0;
				// Diagonal sliding CoP.
				rtInfo.dataSlice[slice].ft[0].force[X] = - 14.0 + 8.5 * s;
				rtInfo.dataSlice[slice].ft[1].force[X] =  - rtInfo.dataSlice[slice].ft[0].force[X];
				rtInfo.dataSlice[slice].ft[0].torque[Y] = rtInfo.dataSlice[slice].ft[0].force[X] * 0.01 * s;
				rtInfo.dataSlice[slice].ft[1].torque[Y] = rtInfo.dataSlice[slice].ft[1].force[X] * 0.011 * s;
				rtInfo.dataSlice[slice].ft[0].torque[Z] = rtInfo.dataSlice[slice].ft[0].force[X] * 0.01 * s;
				rtInfo.dataSlice[slice].ft[1].torque[Z] = rtInfo.dataSlice[slice].ft[1].force[X] * 0.011 * s;
				// Wrist and frame visible.
				rtInfo.dataSlice[slice].markerVisibility[0] = 0xfffff;
				rtInfo.dataSlice[slice].markerVisibility[1] = 0xfffff;
				break;
			}

			// Simulate occaisional occlusions of the manipulandum.
			if ( dropout_count == 0 ) {
				// Note that this simulated value does not take into account 
				// the values of the individal marker visibility set above.
				rtInfo.dataSlice[slice].manipulandumVisibility = true;
				if ( rand() < 1000 ) dropout_count = 10;
			}
			else {
				rtInfo.dataSlice[slice].manipulandumVisibility = false;
				// Here at least we guarantee that marker and manipulandum 
				// visibility flags are coherent.
				rtInfo.dataSlice[slice].markerVisibility[0] &= 0xfff00;
				rtInfo.dataSlice[slice].markerVisibility[1] &= 0xfff00;
				dropout_count --;
			}

		}
		InsertGripRealtimeDataInfo( &rtPacket, &rtInfo );
		ExtractGripRealtimeDataInfo( &reverseInfo, &rtPacket );

		// Send out a realtime data packet.
		iSendResult = send( socket, rtPacket.buffer, rtPacketLengthInBytes, 0 );
		// If we get a socket error it is probably because the client has closed the connection.
		// So we break out of the loop.
		if (iSendResult == SOCKET_ERROR) {
			printf( "RT packet send() failed with error: %3d\n", WSAGetLastError());
			return ( packet_count );
		}
		printf( "  RT packet %3d Bytes sent: %3d\n", packet_count, iSendResult);

		// One HK packet gets sent out for every two real-time data packets. 
		// The BOOL send_hk is used to turn off and on HK output for each RT cycle.
		if ( send_hk ) {

			// Insert the current packet count and time into the packet.
			hkHeaderInfo.TMCounter = packet_count++;
			setPacketTime( &hkHeaderInfo );

			// Set the state of the script interpreter.
			// These constant values have been chosen more or less randomly.
			// I don't make them vary, because they depend on the scripts that are loaded
			//  and it would be too complicated to check whether they are valid values or not.
			hkInfo.user = 11;
			hkInfo.protocol = 201;
			hkInfo.task = 210;
			hkInfo.step = 10;

			// Targets, tone and cradle detectors.
			hkInfo.horizontalTargetFeedback = horizontal_targets;	// Visual targets.
			hkInfo.verticalTargetFeedback = vertical_targets;		
			hkInfo.toneFeedback = sound_bits;						// Tone, muted.
			hkInfo.cradleDetectors = cradle_bits;					// Cradle detectors.
			// Acquisition state.
			hkInfo.motionTrackerStatusEnum = acquire_status;
			hkInfo.crewCameraStatusEnum = camera_status;

			// Insert the housekeeping values into the actual packet and send it out on the socket.
			InsertEPMTelemetryHeaderInfo( &hkPacket, &hkHeaderInfo );
			InsertGripHealthAndStatusInfo( &hkPacket, &hkInfo );
			iSendResult = send( socket, hkPacket.buffer, hkPacketLengthInBytes, 0 );
			// If we get a socket error it is probably because the client has closed the connection.
			// So we break out of the loop.
			if (iSendResult == SOCKET_ERROR) {
				printf( "HK send() failed with error: %3d\n", WSAGetLastError());
				return ( packet_count );
			}
			printf( "  HK packet %3d Bytes sent: %3d\n", packet_count, iSendResult);
		}
		send_hk = !send_hk; // Toggle enable flag so that we do one out of two cycles.

		// Every once in a while, pause a bit to simulate breaks between tasks.
		if ( (packet_count % 20) == 0 ) {
			printf( "\nSimulating inter-trial pause.\n\n" );
			Sleep( 5000 );

			// On each new epoch, fabricate new values
			// for the visible targets, sound generator, cradles and acquisition state.
			// These values stay constant over the course of each epoch.
			vertical_targets = ( 0x01 << ( epoch % N_VERTICAL_TARGETS ) );		// Cycle through vertical targets
			horizontal_targets = ( 0x01 << ( epoch % N_HORIZONTAL_TARGETS ) );	// Cycle through horizontal targets
			sound_bits = epoch % 8;	// Cycle through each tone. Every other tone is 'muted' so every other cycle sound should be off.
			cradle_bits = (epoch % 4) + (((epoch+1) % 4) << 2) + (((epoch+2) % 4) << 4); // Cycle through cradle values, different for each cradle.
			acquire_status = ( epoch % 3 ? 2 : 0 );	// Status 2 means acquiring. Will be so 2 out of 3 epochs.
			camera_status = ( epoch % 2 ? 2 : 0 );	// Status 2 means filming. Will be so 1 out of 2 epochs.
			epoch++;
		}
	}
}

// Send out packets that could come from GRASP.

int sendGraspPackets ( SOCKET socket ) {

	EPMTelemetryPacket hkPacket;
	EPMTelemetryHeaderInfo hkHeaderInfo, rtHeaderInfo;
	GripHealthAndStatusInfo hkInfo;

	static int user = 4;
	static int protocol = 100;
	static int task = 131;
	static int step = 0;
	static int state = 20000;
	static int snapshots = 17;

	static int trials = 7;
	static int status = 321;

	static int packet_count = 0;
    int iSendResult;

	// Prepare the packets by copying constants into local structure.
	memcpy( &hkHeaderInfo, &hkHeader, sizeof( hkHeaderInfo ) );
	memcpy( &rtHeaderInfo, &rtHeader, sizeof( rtHeaderInfo ) );

	while ( 1 ) {

		// RT packets get sent out by GRIP twice per second.
		// This is a trick to avoid drift in the rate.
		// First, make sure that we sleep enough so as not to repeat.
		Sleep( 50 );	
		// Then compute the number of milliseconds to sleep to get back to a 500 ms boundary.
		struct __timeb32 utctime;
		_ftime32_s( &utctime );
		Sleep( (1000 - utctime.millitm ) % 500 );

		if ( rand() > RAND_MAX / 2 ) {
			
			snapshots++;
			
			if ( ( task % 100 ) == 1 ) {
				step++;
				if ( step > 5 ) {
					step = 0;
					task++;
				}
			}
			else {
				step = 0;
				task++;
			}
			if ( task > protocol + 49 ) {
				protocol += 100;
				task = protocol + 1;
			}
		}
			
		state += 10000;
		if ( state == 60000 ) state++;
		if ( state > 60000 ) state = 20000;

		status += 111;
		if ( status > 899) status = 210;

		printf( "User: %d  Protocol: %3d  Task: %3d  Step: %3d   Script: %8d  Tracker: %8d  Snapshots: %3d   ",
			user, protocol, task, step, state, status, snapshots );
		// Insert the current packet count and time into the packet.
		hkHeaderInfo.TMCounter = packet_count++;
		setPacketTime( &hkHeaderInfo );

		// Set the state of the script interpreter.
		// These constant values have been chosen more or less randomly.
		// I don't make them vary, because they depend on the scripts that are loaded
		//  and it would be too complicated to check whether they are valid values or not.
		hkInfo.user = user;
		hkInfo.protocol = protocol;
		hkInfo.task = task;
		hkInfo.step = step;

		// Acquisition state.
		hkInfo.scriptEngineStatusEnum = state;
		hkInfo.motionTrackerStatusEnum = status;
		hkInfo.crewCameraStatusEnum = snapshots;

		

		// Insert the housekeeping values into the actual packet and send it out on the socket.
		InsertEPMTelemetryHeaderInfo( &hkPacket, &hkHeaderInfo );
		InsertGripHealthAndStatusInfo( &hkPacket, &hkInfo );
		iSendResult = send( socket, hkPacket.buffer, hkPacketLengthInBytes, 0 );
		// If we get a socket error it is probably because the client has closed the connection.
		// So we break out of the loop.
		if (iSendResult == SOCKET_ERROR) {
			printf( "HK send() failed with error: %3d\n", WSAGetLastError());
			return ( packet_count );
		}
		printf( "  HK packet %3d Bytes sent: %3d\n", packet_count, iSendResult);
	}
}

// This is the main routine of the executable.
// It parses the command line, initializes a socket to create a CLWS-like server 
// and calls the routine to output packets according to the command line options.

int _tmain( int argc, char **argv )
{
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	// Two possible sources of packets.
	enum { RECORDED_PACKETS, CONSTRUCTED_PACKETS, CONSTRUCTED_GRASP } packet_source = RECORDED_PACKETS;
	// Path to file containing pre-recorded packets. Not used in 'constructed' mode.
	const char *packet_source_filename = DefaultPacketSourceFile;
	// Keep track of how many packets get sent out.
	int packet_count;

	// A place to store raw command packets received from the client.
	EPMTelemetryPacket inputPacket;
	// A place to store the pertinent information from a client packet in usable form.
	EPMTransferFrameHeaderInfo transferFrameInfo;

	printf( "CLWS Emulator started.\n%s\n%s\n\n", GripMMIVersion, GripMMIBuildInfo );
	printf( "This is the EPM/GRIP packet server emulator.\n" );
	printf( "It waits for a client to connect and then sends\n" );
	printf( " out HK and RT packets.\n" );
	printf( "\n\n" );

	// Parse command line.
	// Note here that the project must have "Character Set" option in the project configuration
	//  set to "Not Set" in both the debug and release configurations. Otherwise, the command line
	//  arguments are not passed as the simple (legacy) charcter strings that we expect here.
	for ( int arg = 1; arg < argc; arg++ ) {
		// Provide a visual check to see that the command line arguments are passed correclty.
		// This line helped to debug the character set problem in the project settings.
		printf( "Command Line Argument #%d: %s\n", arg, argv[arg] );
		if ( !strcmp( argv[arg], "-grasp" ) ) packet_source = CONSTRUCTED_GRASP;
		if ( !strcmp( argv[arg], "-constructed" ) ) packet_source = CONSTRUCTED_PACKETS;
		// Playback previously recorded packets.
		else if ( !strcmp( argv[arg], "-recorded" ) ) packet_source = RECORDED_PACKETS;
		// Construct simulated packets.
		else if ( !strncmp( argv[arg], "-port=", strlen( "-port=" ) ) ) EPMport = argv[arg] + strlen( "-port=" );
		else packet_source_filename = argv[arg];
	}	
	if ( packet_source == RECORDED_PACKETS ) {
		printf( "\nSending pre-recorded packets.\n" );
		printf( "Packet source file: %s\n\n", packet_source_filename );
	}
	else if ( packet_source == CONSTRUCTED_PACKETS ) {
		printf( "Constructing simulated packets.\n\n" );
	}

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		printf( "WSAStartup failed with error: %d\n", iResult );
		return 1;
	}
	else if ( _debug ) printf( "WSAStartup() OK.\n" );

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, EPMport, &hints, &result);
	if ( iResult != 0 ) {
		printf("getaddrinfo() failed with error: %d\n", iResult);
		WSACleanup();
		return 2;
	}
	else if ( _debug ) printf( "getaddrinfo() OK.\n" );

	// Create a SOCKET for connecting to client
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket() failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 3;
	}
	else if ( _debug ) printf( "ListenSocket() OK.\n" );

	// Setup the TCP listening socket
	iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind() failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 4;
	}
	else if ( _debug ) printf( "bind() OK.\n" );

	// We don't need the address info anymore, so free it.
	freeaddrinfo(result);

	// Enter an infinite loop that listens for connections,
	//  outputs packets as long as the connection is valid and
	//  then exits. 

	// The only way out is to kill the program (<ctrl-c>).
	// NB We effectively only allow one client at a time.

	while ( 1 ) {

		// Listen until we get a connection.
		printf( "Listening for a connection on port %s ... ", EPMport  );
		iResult = listen(ListenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR) {
			printf("listen() failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 5;
		}
		else if ( _debug ) printf( "listen() OK ... " );

		// Accept a client socket
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf( "accept() failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 6;
		}
		else if ( _debug ) printf( "accept() OK " );
		printf( "connected.\n" );

		// Wait for a 'Connect' command to start sending packets.
		printf( "Waiting for a Connect command ... " );
		do {

			iResult = recv(ClientSocket, inputPacket.buffer, sizeof( inputPacket.buffer ), 0);

			if ( iResult == EPM_BUFFER_LENGTH ) {
				// If we get a full buffer of data, it probably means that we have fallen behind.
				// No packets that we expect from GRIP should use the full EPM buffer length.
				// So just skip this packet and move on to the next.
				printf("Bytes received: %4d - flushing (overrun).\n", iResult);
			}
			else if ( iResult == connectPacketLengthInBytes ) {
				ExtractEPMTransferFrameHeaderInfo( &transferFrameInfo, &inputPacket );
				if ( transferFrameInfo.packetType == TRANSFER_FRAME_CONNECT ) {
					printf("start packet received from ", iResult);
					if ( transferFrameInfo.softwareUnitID == GRIP_MMI_SOFTWARE_UNIT_ID ) printf( "PRIMARY" );
					else if ( transferFrameInfo.softwareUnitID == GRIP_MMI_SOFTWARE_ALT_UNIT_ID ) printf( "ALTERNATE" );
					else printf( "UNRECOGNIZED" );
					printf( " (%d) software unit ID.\n", transferFrameInfo.softwareUnitID );
					break;
				}
				else {
					printf( "unexpected packet type (%x) ... ", transferFrameInfo.packetType );
				}
			}
			else printf( "unexpected packet size (%d) ... ", iResult );

		}while ( iResult > 0 );

		// Send out recorded or artifically constructed packets, depending on a flag set by the command line.
		// The total number of packets sent so far is stored in local variable packetCount.
		switch ( packet_source ) {

		case RECORDED_PACKETS:
			packet_count = sendRecordedPackets( ClientSocket, packet_source_filename );
			break;

		case CONSTRUCTED_PACKETS:
			packet_count = sendConstructedPackets( ClientSocket );
			break;

		case CONSTRUCTED_GRASP:
			packet_count = sendGraspPackets( ClientSocket );
			break;

		}

		// shutdown the connection since we're done
		iResult = shutdown(ClientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			printf( "shutdown() failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 7;
		}
		else if ( _debug ) printf( "shutdown() OK n" );

		printf( "  Total packets sent: %d\n\n", packet_count );

	}

	// cleanup
	closesocket(ListenSocket);
	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}

