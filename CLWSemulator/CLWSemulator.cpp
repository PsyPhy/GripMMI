// CLWSemulator.cpp : Defines the entry point for the console application.
//

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
#include "..\Grip\GripPackets.h"
#include "..\GripMMIVersionControl\GripMMIVersionControl.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

PCSTR EPMport = EPM_DEFAULT_PORT;

EPMTelemetryHeaderInfo hkHeaderInfo, rtHeaderInfo;
EPMTelemetryPacket hkPacket, rtPacket;
EPMTelemetryPacket inputPacket;
EPMTelemetryPacket recordedPacket;

// Path to a file containing a mixture of different packet types.
// These packets were stored during a real (albeit abbreviated) Grip sesion.
// For the moment, this path is hard coded. Perhaps there should be a way
//  to change it, but as this program is not a deliverable, it is not high priority.
char *PacketSourceFile = ".\\GripPacketsForSimulator.gpk";

#ifdef _DEBUG
	bool _debug = true;
#else
	bool _debug = false;
#endif

BOOL verbose = false;


void setPacketTime( EPMTelemetryHeaderInfo *header ) {

	// Set the time of an EPM telemetry packet.

	// Time structure to get the local time for the EPM packet coarse and fine time values.
	// I am using the 32 bit version because the EPM coarse time is 32 bits.
	struct __timeb32 epmtime;
	_ftime32_s( &epmtime );

	// NB EPM uses GPS time (second since midnight Jan 5-6 1980), while 
	// _ftime_s() uses seconds since midnight Jan. 1 1970 UTC.
	// I am just using UTC time, since it doesn't really matter for 
	// my purposes here. The idea is just to keep the packets in the right order.
	// Also, EPM somehow gets time in 10ths of milliseconds and puts that in the header. 
	// We don't expect to get two packet in a span of less than a millisecond, so I don't worry about it.

	// One could probably treat coarseTime as an unsigned long in the
	// header and just copy the 32-bit value, but I'm not sure about word order. 
	// So to be sure to match the EPM spec as it is written, I transfer each
	//  two-bye word separately.
	header->coarseTime = epmtime.time;
	// Compute the fine time in 10ths of milliseconds.
	header->fineTime = epmtime.millitm * 10;

}

// This is the routine that sends out packets that were pre-recorded.
void sendRecordedPackets ( SOCKET socket ) {

	static int packetCount = 0;
    int iSendResult;
	int sendHK = 1;

	int	fid;
	int return_code;
	int bytes_read;
			
	EPMTelemetryHeaderInfo epmPacketHeaderInfo;

	while ( 1 ) {

		printf( "Sending out recorded packets:\n\n  %s\n\n", PacketSourceFile );

		// Open the file where the packets are stored.
		return_code = _sopen_s( &fid, PacketSourceFile, _O_RDONLY | _O_BINARY, _SH_DENYNO, _S_IWRITE | _S_IREAD  );
		if ( return_code ) {
			fMessageBox( MB_OK, "CLWSemulator", "Error opening %s for binary read.", PacketSourceFile );
			exit( -1 );
		}
		// Read the first one.
		while ( bytes_read = _read( fid, recordedPacket.buffer, sizeof( recordedPacket.buffer ) ) ) {
			if ( bytes_read < 0 ) {
				fMessageBox( MB_OK, "CLWSemulator", "Error reading from %s.", PacketSourceFile );
				exit( -1 );
			}
			ExtractEPMTelemetryHeaderInfo( &epmPacketHeaderInfo, &recordedPacket );
			if ( epmPacketHeaderInfo.epmSyncMarker != EPM_TELEMETRY_SYNC_VALUE ) {
				// If it's not an EPM packet we don't send it out.
				if ( verbose ) printf( "Bytes: %4d (non EPM).\n", bytes_read ); 
			}
			else {
				if ( verbose ) printf( "Bytes: %4d EPM.\n", bytes_read );
				if ( epmPacketHeaderInfo.subsystemID != GRIP_SUBSYSTEM_ID ) printf( "." );
				else {
					printf( "G" );
					// Set the timestamp of the packet to the current time.
					setPacketTime( &epmPacketHeaderInfo );
					// Set the packet counter based on a local count.
					epmPacketHeaderInfo.TMCounter = packetCount++;
					// Put the new header info back into the packet.
					InsertEPMTelemetryHeaderInfo( &rtPacket, &epmPacketHeaderInfo );
					iSendResult = send( socket, recordedPacket.buffer, EPM_BUFFER_LENGTH - 1, 0 );
					// If we get a socket error it is probably because the client has closed the connection.
					// So we break out of the loop.
					if (iSendResult == SOCKET_ERROR) {
						fprintf( stderr, "Recorded packet send failed with error: %3d\n", WSAGetLastError());
						return;
					}
					// What we should do here is sleep based on the difference in time between the previous
					//  recorded packet and this one. For now we just sleep and let it run fast.
					Sleep( 400 );
				}
			}
		}
		return_code = _close( fid );
		if ( return_code ) {
			fMessageBox( MB_OK, "CLWSemulator", "Error closing %s after binary read.\nError code: %s", PacketSourceFile, return_code );
			exit( return_code );
		}

		printf( "Playback completed. Will restart in 10 seconds.\n" );
		Sleep( 10000 );

	}

}
void sendConstructedPackets ( SOCKET socket ) {

	int packetCount = 0;
    int iSendResult;
	int sendHK = 1;

	// Prepare the packets by copying constants into local structure.
	memcpy( &hkHeaderInfo, &hkHeader, sizeof( hkHeaderInfo ) );
	memcpy( &rtHeaderInfo, &rtHeader, sizeof( rtHeaderInfo ) );

	// Send packets until the peer shuts down the connection
	while ( 1 ) {

		// RT packets get sent out by GRIP twice per second.
		// This is a trick to avoid drift in the rate.
		// We compute the number of milliseconds to sleep to get back to a 500 ms boundary.
		Sleep( 10 );	
		struct __timeb32 utctime;
		_ftime32_s( &utctime );
		Sleep( (1000 - utctime.millitm ) % 500 );

		// Insert the current packet count and time into the packet.
		rtHeaderInfo.TMCounter = packetCount++;
		setPacketTime( &rtHeaderInfo );
		InsertEPMTelemetryHeaderInfo( &rtPacket, &rtHeaderInfo );
		// Now in theory we should fill it with something, but I don't do anything yet.
		// Send out a realtime data packet.
		iSendResult = send( socket, rtPacket.buffer, rtPacketLengthInBytes, 0 );
		// If we get a socket error it is probably because the client has closed the connection.
		// So we break out of the loop.
		if (iSendResult == SOCKET_ERROR) {
			fprintf( stderr, "RT packet send failed with error: %3d\n", WSAGetLastError());
			return;
		}
		fprintf( stderr, "  RT packet %3d Bytes sent: %3d\n", packetCount, iSendResult);

		// HK packets get sent once every 2 seconds. 
		// The BOOL sendHK is used to turn off and on for each RT cycle.
		if ( sendHK ) {
			// Insert the current packet count and time into the packet.
			hkHeaderInfo.TMCounter = packetCount++;
			setPacketTime( &hkHeaderInfo );
			InsertEPMTelemetryHeaderInfo( &hkPacket, &hkHeaderInfo );
			// Send out a housekeeping packet.
			iSendResult = send( socket, hkPacket.buffer, hkPacketLengthInBytes, 0 );
			// If we get a socket error it is probably because the client has closed the connection.
			// So we break out of the loop.
			if (iSendResult == SOCKET_ERROR) {
				fprintf( stderr, "HK send failed with error: %3d\n", WSAGetLastError());
				return;
			}
			fprintf( stderr, "  HK packet %3d Bytes sent: %3d\n", packetCount, iSendResult);
		}
		sendHK = !sendHK; // Toggle enable flag so that we do one out of two cycles.

		// Every once in a while, pause a bit to simulate breaks between tasks.
		if ( (packetCount % 20) == 0 ) {
			fprintf( stderr, "\nSimulating inter-trial pause.\n\n" );
			Sleep( 5000 );
		}
	}
}

void sendPrimingPackets ( SOCKET socket ) {

	int packetCount = 0;
    int iSendResult;
	int slice;

	GripHealthAndStatusInfo hkInfo;
	GripRealtimeDataInfo rtInfo;

	// Prepare the packets by copying constants into local structure.
	memcpy( &hkHeaderInfo, &hkHeader, sizeof( hkHeaderInfo ) );
	memcpy( &rtHeaderInfo, &rtHeader, sizeof( rtHeaderInfo ) );

	// Send just one packet of each type;

	// Insert the current packet count and time into the packet.
	rtHeaderInfo.TMCounter = packetCount++;
	setPacketTime( &rtHeaderInfo );
	InsertEPMTelemetryHeaderInfo( &rtPacket, &rtHeaderInfo );

	// Fill in null data.
	rtInfo.acquisitionID = 0;
	rtInfo.rtPacketCount = packetCount++;
	for ( slice = 0; slice < RT_SLICES_PER_PACKET; slice++ ) {
		int i, unit;

		rtInfo.dataSlice[slice].poseTick = 0;
		for ( i = X; i <=Z; i++ ) rtInfo.dataSlice[slice].position[i] = 0.0;
		for ( i = X; i <=M; i++ ) rtInfo.dataSlice[slice].quaternion[i] = 0.0;
		for ( i = 0; i < 2; i++ ) rtInfo.dataSlice[slice].markerVisibility[i] = 0x00000000;
		rtInfo.dataSlice[slice].manipulandumVisibility = 0;
		rtInfo.dataSlice[slice].analogTick = 0;
		for ( unit = 0; unit < 2; unit++ ) {
			for ( i = X; i <=Z; i++ ) rtInfo.dataSlice[slice].ft[unit].force[i] = 0.0;
			for ( i = X; i <=Z; i++ ) rtInfo.dataSlice[slice].ft[unit].torque[i] = 0.0;
		}
		for ( i = X; i <=Z; i++ ) rtInfo.dataSlice[slice].acceleration[i] = 0.0;
	}
	InsertGripRealtimeDataInfo( &rtPacket, &rtInfo );

	// Send out a realtime data packet.
	iSendResult = send( socket, rtPacket.buffer, rtPacketLengthInBytes, 0 );
	// If we get a socket error it is probably because the client has closed the connection.
	// So we break out of the loop.
	if (iSendResult == SOCKET_ERROR) {
		fprintf( stderr, "RT packet send failed with error: %3d\n", WSAGetLastError());
		return;
	}
	fprintf( stderr, "  RT packet %3d Bytes sent: %3d\n", packetCount, iSendResult);
	Sleep( 200 );

	// Insert the current packet count and time into the packet.
	hkHeaderInfo.TMCounter = packetCount++;
	setPacketTime( &hkHeaderInfo );
	InsertEPMTelemetryHeaderInfo( &hkPacket, &hkHeaderInfo );
	// Give user, protocol, task and step info as if the subject has already started a task.
	// This is to overcome a bug in the MMI. It will be removed later.
	hkInfo.user = 11;
	hkInfo.protocol = 201;
	hkInfo.task = 201;
	hkInfo.step = 0;

	hkInfo.horizontalTargetFeedback = 0;	// No targets on.
	hkInfo.verticalTargetFeedback = 0;		
	hkInfo.toneFeedback = 0x01;				// Lowest tone, muted.
	hkInfo.cradleDetectors = 0x01B;			// S, M, L, I think.

	InsertGripHealthAndStatusInfo( &hkPacket, &hkInfo );

	// Send out a housekeeping packet.
	iSendResult = send( socket, hkPacket.buffer, hkPacketLengthInBytes, 0 );
	// If we get a socket error it is probably because the client has closed the connection.
	// So we break out of the loop.
	if (iSendResult == SOCKET_ERROR) {
		fprintf( stderr, "HK send failed with error: %3d\n", WSAGetLastError());
		return;
	}
	fprintf( stderr, "  HK packet %3d Bytes sent: %3d\n", packetCount, iSendResult);

	Sleep( 1000 );

}
int _tmain(int argc, char* argv[])
{

	WSADATA wsaData;
	int iResult;

	int packetCount = 0;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	enum { RECORDED_PACKETS, CONSTRUCTED_PACKETS, PRIMING_PACKETS } packet_source = RECORDED_PACKETS;
	int arg;

	EPMTransferFrameHeaderInfo transferFrameInfo;

	fprintf( stderr, "CLWS Emulator started.\n%s\n%s\n\n", GripMMIVersion, GripMMIBuildInfo );
	fprintf( stderr, "This is the EPM/GRIP packet server emulator.\n" );
	fprintf( stderr, "It waits for a client to connect and then sends\n" );
	fprintf( stderr, " out HK and RT packets.\n" );
	fprintf( stderr, "\n" );

	// Parse command line.
	for ( arg = 1; arg < argc; arg++ ) {
		// Playback previously recorded packets.
		if ( !strcmp( argv[arg], "-recorded" ) ) packet_source = RECORDED_PACKETS;
		// Construct packets with sine waves and such.
		if ( !strcmp( argv[arg], "-constructed" ) ) packet_source = CONSTRUCTED_PACKETS;
		// Write out single null packets to prime the pump.
		// This allows the GripGroundMonitorClient to start up.
		if ( !strcmp( argv[arg], "-prime" ) ) packet_source = PRIMING_PACKETS;
	}	
	if ( packet_source == RECORDED_PACKETS ) fprintf( stderr, "Sending pre-recorded packets.\n\n" );
	else if ( packet_source == CONSTRUCTED_PACKETS ) fprintf( stderr, "Constructing simulated packets.\n\n" );
	else if ( packet_source == PRIMING_PACKETS ) fprintf( stderr, "Sending priming packets.\n\n" );

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		fprintf( stderr, "WSAStartup failed with error: %d\n", iResult );
		return 1;
	}
	else if ( _debug ) fprintf( stderr, "WSAStartup() OK.\n" );

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, EPMport, &hints, &result);
	if ( iResult != 0 ) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 2;
	}
	else if ( _debug ) fprintf( stderr, "getaddrinfo() OK.\n" );

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 3;
	}
	else if ( _debug ) fprintf( stderr, "ListenSocket() OK.\n" );

	// Setup the TCP listening socket
	iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 4;
	}
	else if ( _debug ) fprintf( stderr, "bind() OK.\n" );

	// We don't need the address info anymore, so free it.
	freeaddrinfo(result);

	// Enter an infinite loop that listens for connections,
	//  outputs packets as long as the connection is valid and
	//  then exits. 
	// The only way out is to kill the program (<ctrl-c>).
	// NB We effectively only allow one client at a time.

	while ( 1 ) {

		// Listen until we get a connection.
		fprintf( stderr, "Listening for a connection ... " );
		iResult = listen(ListenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR) {
			printf("listen failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 5;
		}
		else if ( _debug ) fprintf( stderr, "listen() OK " );

		// Accept a client socket
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			fprintf( stderr, "accept failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 6;
		}
		else if ( _debug ) fprintf( stderr, "acceot() OK " );
		fprintf( stderr, "connected.\n" );

		// Wait for a 'Connect' command to start sending packets.
		fprintf( stderr, "Waiting for a Connect command ... " );
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
					printf("start packet received.\n", iResult);
					break;
				}
				else {
					printf( "unexpected packet type (%x) ... ", transferFrameInfo.packetType );
				}
			}
			else printf( "unexpected packet size (%d) ... ", iResult );

		}while ( iResult > 0 );

		// Send out recorded or artifically constructed packets, depending on a flag.
		switch ( packet_source ) {

		case RECORDED_PACKETS:
			sendRecordedPackets( ClientSocket );
			break;

		case CONSTRUCTED_PACKETS:
			sendConstructedPackets( ClientSocket );
			break;

		case PRIMING_PACKETS:
			sendPrimingPackets( ClientSocket );
			break;

		}

		// shutdown the connection since we're done
		iResult = shutdown(ClientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			fprintf( stderr, "shutdown() failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 7;
		}
		else if ( _debug ) fprintf( stderr, "shutdown() OK n" );

		fprintf( stderr, "  Total packets sent: %d\n\n", hkHeaderInfo.TMCounter + rtHeaderInfo.TMCounter );
		hkHeaderInfo.TMCounter = 0;
		rtHeaderInfo.TMCounter = 0;

	}

	// cleanup
	closesocket(ListenSocket);
	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}

