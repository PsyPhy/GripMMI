///
/// Module:	GripGroundMonitorClient (GripMMI)
/// 
///	Author:					J. McIntyre, PsyPhy Consulting
/// Initial release:		18 December 2014
/// Modification History:	see https://github.com/PsyPhy/GripMMI
///
/// Copyright (c) 2014, 2015 PsyPhy Consulting
///

/// Connection to EPM for the GripMMI.
/// This file creates a console application that connects to the EPM CLWS server.
/// It accepts GRIP housekeeping and realtime data packets and stores them in
///  cache files. Those cache files are then be read by the GripMMI executable
///  for graphical display. 

#include "stdafx.h"
#include "..\Grip\GripPackets.h"
#include "..\Useful\fMessageBox.h"
#include "..\Useful\fOutputDebugString.h"
#include "..\GripMMIVersionControl\GripMMIVersionControl.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

PCSTR EPMport = EPM_DEFAULT_PORT;
EPMTelemetryPacket epmPacket;
EPMTelemetryHeaderInfo epmPacketHeaderInfo;

// Buffers to hold the path to the packet caches.
char rtPacketCacheFilePath[1024];
char hkPacketCacheFilePath[1024];
char anyPacketCacheFilePath[1024];

// Count the number of packets of each type sent to the cache files.
unsigned long rtCount = 0;
unsigned long hkCount = 0;
unsigned long anyCount = 0;


void outputPacket( EPMTelemetryPacket *packet, int n_bytes, const char *filename ) {

	int		fid;
	errno_t	return_code;
	size_t	bytes_written;

	return_code = _sopen_s( &fid, filename, _O_CREAT | _O_WRONLY | _O_APPEND | _O_BINARY, _SH_DENYWR, _S_IREAD | _S_IWRITE );
	if ( return_code ) {
		fMessageBox( MB_OK, "GripGroundMonitorClient", "Error opening %s for binary write.\nError code: %d", filename, return_code );
		exit( return_code );
	}
	bytes_written = _write( fid, packet, n_bytes );
	if ( bytes_written != n_bytes ) {
		fMessageBox( MB_OK, "GripGroundMonitorClient", "Error writing to %s.", filename  );
		exit( -1 );
	}
	return_code = _close( fid );
	if ( return_code ) {
		fMessageBox( MB_OK, "GripGroundMonitorClient", "Error closing %s after binary write.\nError code: %d", filename, return_code );
		exit( return_code );
	}

}

void outputHK ( EPMTelemetryPacket *packet ) {
	outputPacket( packet, hkPacketLengthInBytes, hkPacketCacheFilePath  );
	hkCount++;
}
void outputRT ( EPMTelemetryPacket *packet ) {
	outputPacket( packet, rtPacketLengthInBytes, rtPacketCacheFilePath );
	rtCount++;
}
void outputANY ( EPMTelemetryPacket *packet ) {
	outputPacket( packet, EPM_BUFFER_LENGTH, anyPacketCacheFilePath );
	anyCount++;
}

int __cdecl main(int argc, char **argv) 
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    int iResult;

	struct __timeb32 utctime;
	long	previous_alive_time = 0;
	bool	verbose = true;
	bool	cache_all = true;
	bool	use_alt_id = false;
	int		software_unit_id = GRIP_MMI_SOFTWARE_UNIT_ID;

	char *packetCacheFilenameRoot = NULL;
	char *server_name = NULL;

	fprintf( stderr, "GripGroundMonitorClient started.\n%s\n%s\n\n", GripMMIVersion, GripMMIBuildInfo );
	fprintf( stderr, "This is the EPM/GRIP packet receiver.\n" );
	fprintf( stderr, "It waits for a connection to the EPM server,\n then processes incoming packets.\n" );
	fprintf( stderr, "\n" );

	// Parse the command line.
	for ( int arg = 1; arg < argc; arg ++ ) {

		if ( !strcmp( argv[arg], "-alt" )) use_alt_id = true;
		else if ( !strcmp( argv[arg], "-only" )) cache_all = false;
		else if ( packetCacheFilenameRoot == NULL ) {
			packetCacheFilenameRoot = argv[arg];
			printf( "Using command-line packet output root: %s\n", packetCacheFilenameRoot );
		}
		else if ( server_name == NULL ) {
			server_name = argv[arg];
			printf( "Using command-line server name: %s\n", server_name );
		}
		else printf( "Too many command line arguments (%s)\n", argv[arg] );
	}
	if (packetCacheFilenameRoot == NULL ) {
		packetCacheFilenameRoot = ".\\";
		printf( "Using default output root: %s\n", packetCacheFilenameRoot );
	}
	if ( server_name == NULL ) {
		server_name = "localhost";
		printf( "Using default server name: %s\n", server_name );
	}
	if ( cache_all ) fprintf( stderr, "Saving all packets.\n" );
	else fprintf( stderr, "Saving only GRIP packets.\n" );
	if ( use_alt_id ) {
		software_unit_id = GRIP_MMI_SOFTWARE_ALT_UNIT_ID;
		printf( "Using alternate Software Unit ID.\n" );
	}
	fprintf( stderr, "Software Unit ID: %d\n", software_unit_id );
	connectPacket.softwareUnitID = software_unit_id;
	alivePacket.softwareUnitID = software_unit_id;

	fprintf( stderr, "\n" );

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 102;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo( server_name, EPMport, &hints, &result );
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 103;
    }

    // Attempt to connect to an address until one succeeds
	printf( "Waiting for connection with host %s on port %s.\n", server_name, EPMport );
	printf( "Will wait until connection achieved or <ctrl-C>.\n"  );

	while ( 1 ) {
		for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {
			// Create a SOCKET for connecting to server
			ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if (ConnectSocket == INVALID_SOCKET) {
				printf("socket failed with error: %ld\n", WSAGetLastError());
				WSACleanup();
				return 104;
			}
			// Try to connect to server.
			iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			// If no error, we got a connection. 
			// Break out of the loop to use this connection.
			if (iResult != SOCKET_ERROR) break;
			// Otherwise, try the next element in the list returned by getaddrinfo.
			else {
				closesocket(ConnectSocket);
				ConnectSocket = INVALID_SOCKET;
			}
		}
		// If we get here, it's either because we managed to connect or
		//  because there are no more items to try. In the latter case,
		//  ConnectSocket will have the value INVALID_SOCKET.
		// If we have a valid connect break out of the loop that is waiting for one.
		if ( ConnectSocket != INVALID_SOCKET ) break;
		// Otherwise, show some progress and loop back for another try.
		else printf( "." );
	}

	// We have a connection. 
	printf( "\nConnection established with server.\n" );
	printf( "Sending Connect command.\n" );
	InsertEPMTransferFrameHeaderInfo( &epmPacket, &connectPacket );
	iResult = send( ConnectSocket, epmPacket.buffer, connectPacketLengthInBytes, 0 );
	// If we get a socket error it is probably because the client has closed the connection.
	// So we break out of the loop.
	if ( iResult == SOCKET_ERROR ) {
		fprintf( stderr, "Command packet send failed with error: %3d\n", WSAGetLastError());
		getchar();
		exit( -100 );
	}
	else fprintf( stderr, "Command packet bytes sent: %3d\n\n", iResult);

	// We no longer need the address info.
    freeaddrinfo(result);

	// We have a connection and are ready to start receiving packets.
	// Create the file names that will hold the packets. 
	// The filenames are based on today's date.
	CreateGripPacketCacheFilename( hkPacketCacheFilePath, sizeof( hkPacketCacheFilePath ), GRIP_HK_BULK_PACKET,    packetCacheFilenameRoot );
	fprintf( stderr, "Output HK packets to: %s\n", hkPacketCacheFilePath );
	CreateGripPacketCacheFilename( rtPacketCacheFilePath, sizeof( rtPacketCacheFilePath ), GRIP_RT_SCIENCE_PACKET, packetCacheFilenameRoot );
	fprintf( stderr, "Output RT packets to: %s\n", rtPacketCacheFilePath );
	if ( cache_all ) {
		CreateGripPacketCacheFilename( anyPacketCacheFilePath, sizeof( anyPacketCacheFilePath ), GRIP_UNKNOWN_PACKET, packetCacheFilenameRoot );
		fprintf( stderr, "Output ALL packets to: %s\n", anyPacketCacheFilePath );
	}
	fprintf( stderr, "\n" );

	// Receive as long as the server stays connected or until <ctrl-C>.
    do {

        iResult = recv(ConnectSocket, epmPacket.buffer, EPM_BUFFER_LENGTH, 0);

		if ( iResult == EPM_BUFFER_LENGTH ) {

			// If we get a full buffer of data, it probably means that we have fallen behind.
			// No packets that we expect from GRIP should use the full EPM buffer length.
			// So just skip this packet and move on to the next.
            printf("Bytes: %4d - flushing (overrun).\n", iResult);
		}
        else if ( iResult > 0 ) {

			// Collect all packets, according to the command line flag.
			if ( cache_all ) outputANY( &epmPacket );
			
			// Get the EPM header info and process the packet according to the type.
			// First check for the EPM sync words and discard if not valid.
			ExtractEPMTelemetryHeaderInfo( &epmPacketHeaderInfo, &epmPacket );
			if ( epmPacketHeaderInfo.epmSyncMarker != EPM_TELEMETRY_SYNC_VALUE ) {
				if ( verbose ) printf( "Bytes: %4d (non EPM).\n", iResult ); 
			}
			else {
				// Check that the packet came from GRIP.
				if ( epmPacketHeaderInfo.subsystemID != GRIP_SUBSYSTEM_ID ) {
					if ( verbose ) printf( "Bytes: %4d %4d %4d %02x:%02x:%02x TM: 0x%04x %06d (non GRIP).\n",

						iResult, 
						epmPacketHeaderInfo.transferFrameInfo.numberOfWords * 2, 
						epmPacketHeaderInfo.numberOfWords * 2, 

						epmPacketHeaderInfo.transferFrameInfo.softwareUnitID,
						epmPacketHeaderInfo.subsystemID, 
						epmPacketHeaderInfo.subsystemUnitID, 

						epmPacketHeaderInfo.TMIdentifier, 
						epmPacketHeaderInfo.TMCounter
						);
				}
				else {
					printf( "Bytes: %4d %4d %4d %02x:%02x:%02x TM: 0x%04x %06d",
						
						iResult,													// Actual # bytes received.
						epmPacketHeaderInfo.transferFrameInfo.numberOfWords * 2,	// Bytes supposedly received according to transfer frame header.
						epmPacketHeaderInfo.numberOfWords * 2,						// Bytes supposedly recieved according to the EPM Telemetry packet, excluding transfer frame info.  
						
						epmPacketHeaderInfo.transferFrameInfo.softwareUnitID,
						epmPacketHeaderInfo.subsystemID, 
						epmPacketHeaderInfo.subsystemUnitID, 
						
						epmPacketHeaderInfo.TMIdentifier,
						epmPacketHeaderInfo.TMCounter
					);
					// Then check the type of EPM packet and sort into appropriate cache files.
					// We are only concerned with two packet types: 
					//   0x0301 for housekeeping data and 0x1001 for realtime science data.
					switch ( epmPacketHeaderInfo.TMIdentifier ) {

					case GRIP_HK_ID:
						printf( " HK   \n" );
						outputHK( &epmPacket );
						break;

					case GRIP_RT_ID:
						printf( "    RT\n" );
						outputRT( &epmPacket );
						break;

					default:
						// It would be surprising to get here as it would
						//  mean that GRIP sent an unexpected packet type.
						printf( " ??????\n" );
						break;

					}
				}
			}
		}

		// Every second or so we should send an Alive command to the server.
		_ftime32_s( &utctime );
		if ( utctime.time > previous_alive_time ) {
			previous_alive_time = utctime.time;
			// printf( "Sending Alive command.\n" );
			InsertEPMTransferFrameHeaderInfo( &epmPacket, &alivePacket );
			iResult = send( ConnectSocket, epmPacket.buffer, alivePacketLengthInBytes, 0 );
			// If we get a socket error it is probably because the client has closed the connection.
			// So we break out of the loop.
			if ( iResult == SOCKET_ERROR ) {
				fprintf( stderr, "Alive packet send failed with error: %3d\n", WSAGetLastError());
				getchar();
				exit( -100 );
			}
			// else fprintf( stderr, "Alive packet bytes sent: %3d\n", iResult);
		}

	// Keep looping as long as we are receiving packets.
    } while( iResult > 0 ); // End loop if connection is closed or on error.
	
	// Show what caused us to exit the receiver loop.
	if ( iResult == 0 )printf("\nConnection closed by host.\n");
    else printf("\nrecv failed with error: %d\n", WSAGetLastError());

    // In this version, if the server closes the connection, we quit as well.
	// In a future versions, we could go back to listen for a connection again.
    closesocket(ConnectSocket);
    WSACleanup();

	// Make sure that the user sees the final message by requiring a keyboard input.
	printf( "Press <return>\n" );
	getchar();

	return 0;
}

