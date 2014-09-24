// GripGroundMonitorClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\Grip\GripPackets.h"
#include "..\Useful\fMessageBox.h"
#include "..\Useful\fOutputDebugString.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

PCSTR EPMport = EPM_DEFAULT_PORT;
EPMTelemetryPacket epmPacket;
EPMTelemetryHeaderInfo epmPacketHeaderInfo;

// Default path for packet storage is the current directory.
char *destination_directory = ".\\";

// Buffers to hold the path to the packet caches.
char rtPacketOutputFilePath[1024];
char hkPacketOutputFilePath[1024];
char anyPacketOutputFilePath[1024];

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
	printf( "    Appended to %s.\n", filename );

}

void outputHK ( EPMTelemetryPacket *packet ) {
	outputPacket( packet, hkPacketLengthInBytes, hkPacketOutputFilePath  );
	hkCount++;
}
void outputRT ( EPMTelemetryPacket *packet ) {
	outputPacket( packet, rtPacketLengthInBytes, rtPacketOutputFilePath );
	rtCount++;
}
void outputANY ( EPMTelemetryPacket *packet ) {
	outputPacket( packet, EPM_BUFFER_LENGTH, anyPacketOutputFilePath );
	anyCount++;
}

int __cdecl main(int argc, char **argv) 
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    int iResult;

	// Parse the command line.
	char *server_name = "localhost";
	if ( argc < 2 ) printf( "Using default server name: %s\n", server_name );
	else {
		server_name = argv[1];
		printf( "Using command-line server name: %s\n", server_name );
	}
	if ( argc < 3 ) printf( "Using default output path: %s\n", destination_directory );
	else {
		destination_directory = argv[2];
		printf( "Using command-line packet output directory: %s\n", destination_directory );
	}

	fprintf( stderr, "This is the EPM/GRIP packet receiver.\n" );
	fprintf( stderr, "It waits for a connection to the EPM server,\n then processes incoming packets.\n" );
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
	printf( "\nConnection established with server.\n\n" );

	// We no longer need the address info.
    freeaddrinfo(result);

	// We have a connection and are ready to start receiving packets.
	// Create the file names that will hold the packets. 
	// The filenames are based on today's date.
	SYSTEMTIME	systime;
	int	bytes_written;
	GetSystemTime( &systime );
	bytes_written = sprintf_s( rtPacketOutputFilePath, sizeof( rtPacketOutputFilePath ), "%s\\RT.%04d.%02d.%02d.pkt", 
		destination_directory, systime.wYear, systime.wMonth, systime.wDay );
	if ( bytes_written < 0 ) {
			OutputDebugString( "Error in sprintf().\n" );
			exit( -1 );
	}
	bytes_written = sprintf_s( hkPacketOutputFilePath, sizeof( hkPacketOutputFilePath ), "%s\\HK.%04d.%02d.%02d.pkt", 
		destination_directory, systime.wYear, systime.wMonth, systime.wDay );
	if ( bytes_written < 0 ) {
			OutputDebugString( "Error in sprintf().\n" );
			exit( -1 );
	}
	bytes_written = sprintf_s( anyPacketOutputFilePath, sizeof( hkPacketOutputFilePath ), "%s\\ANY.%04d.%02d.%02d.pkt", 
		destination_directory, systime.wYear, systime.wMonth, systime.wDay );
	if ( bytes_written < 0 ) {
			OutputDebugString( "Error in sprintf().\n" );
			exit( -1 );
	}

	// Receive as long as the server stays connected or until <ctrl-C>.
    do {

        iResult = recv(ConnectSocket, epmPacket.buffer, EPM_BUFFER_LENGTH, 0);

		if ( iResult == EPM_BUFFER_LENGTH ) {
			// If we get a full buffer of data, it probably means that we have fallen behind.
			// No packets that we expect from GRIP should use the full EPM buffer length.
			// So just skip this packet and move on to the next.
            printf("Bytes received: %4d - flushing (overrun).\n", iResult);
			// Write it out anyway to the all-packet cache for debugging.
			outputANY( &epmPacket );
		}
        else if ( iResult > 0 ) {
            printf("Bytes: %4d", iResult);
			
			// Write it out to the all-packet cache for debugging.
 			outputANY( &epmPacket );
			// Get the EPM header info and process the packet according to the type.
			// First check for the EPM sync words and discard if not valid.
			ExtractEPMTelemetryHeaderInfo( &epmPacketHeaderInfo, &epmPacket );
			if ( epmPacketHeaderInfo.epmSyncMarker != EPM_TELEMETRY_SYNC_VALUE ) printf( " - flushing (non EPM).\n" ); 
			else printf( " TM Code 0x%04x  TM Counter %03d EPM Time %lf\n", epmPacketHeaderInfo.TMIdentifier, epmPacketHeaderInfo.TMCounter, EPMtoSeconds( epmPacketHeaderInfo ) );
			// Then check the type of EPM packet and sort into appropriate cache files.
			// We are only concerned with two packet type: 
			//   0x0301 for housekeeping data and 0x1001 for realtime science data.
			switch ( epmPacketHeaderInfo.TMIdentifier ) {

			case GRIP_HK_ID:
				printf( "    Processing House Keeping Packet.\n" );
				outputHK( &epmPacket );
				printf( "    Realtime Data Packets: %8u\n", hkCount );
				break;

			case GRIP_RT_ID:
				printf( "    Processing Realtime Data Packet.\n" );
				outputRT( &epmPacket );
				break;

			default:
				printf( "    discardng unrecognized EPM packet.\n" );
				break;

			}
			printf( "    Packets: %8u  %8u\n", rtCount, hkCount );
		}

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

