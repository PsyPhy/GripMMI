// GripGroundMonitorClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\\..\\GRIPserver2\GRIPserver2\\GripPackets.h"

PCSTR EPMport = EPM_DEFAULT_PORT;
EPMTelemetryPacket epmPacket;

// Default path for packet storage is the current directory.
char *destination_directory = ".\\";

// Buffers to hold the path to the packet caches.
char rtPacketOutputFilePath[1024];
char hkPacketOutputFilePath[1024];

/***********************************************************************************/

// Some useful routines that I like to have around. Usually I would put them in 
//  a library so as to share them between projects, but I don't know how to do that
//  yet in VC++ 2010.

// I get some warnings about some functions that might not be safe. I am continuing 
//  to use the classical versions because I don't see that the others are that much
//  safer. But I can change them is required.

// Make it easier to construct messages and display them as a MessageBox.
int fMessageBox( int mb_type, const char *caption, const char *format, ... ) {
	
	int items;
	
	va_list args;
	
	// The character buffer is really long so that there is little chance of overrunning it.
	char message[10240];
	
	va_start(args, format);
	items = vsprintf(message, format, args);
	va_end(args);
	
	return( MessageBox( NULL, message, caption, mb_type ) );
		
}
// Make it easier to construct messages to output in the debug window.
int fOutputDebugString( const char *format, ... ) {

	int items;

	va_list args;
	// The character buffers are really long so that there is little chance of overrunning.
	char message[10240];
	char fmt[10240];

	strcpy( fmt, "*** DEBUG OUTPUT: " );
	strcat( fmt, format );
	va_start(args, format);
	items = vsprintf(message, fmt, args);
	va_end(args);

	OutputDebugString( message );

	return( items );

}

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
}
void outputRT ( EPMTelemetryPacket *packet ) {
	outputPacket( packet, rtPacketLengthInBytes, rtPacketOutputFilePath );
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

	// Receive as long as the server stays connected or until <ctrl-C>.
    do {

        iResult = recv(ConnectSocket, epmPacket.buffer, EPM_BUFFER_LENGTH, 0);
		if ( iResult == EPM_BUFFER_LENGTH ) {
			// If we get a full buffer of data, it probably means that we have fallen behind.
			// No packets that we expect from GRIP should use the full EPM buffer length.
			// So just skip this packet and move on to the next.
             printf("Bytes received: %4d - flushing (overrun).\n", iResult);
		}
        else if ( iResult > 0 ) {
            printf("Bytes: %4d", iResult);
			if ( epmPacket.header.epmSyncMarker != EPM_TELEMETRY_SYNC_VALUE ) printf( " - flushing (non EPM).\n" ); 
			else printf( " TM Code 0x%04x  TM Counter %03d EPM Time %lf\n", epmPacket.header.TMIdentifier, epmPacket.header.TMCounter, EPMtoSeconds( epmPacket ) );
			switch ( epmPacket.header.TMIdentifier ) {
			case GRIP_HK_ID:
				printf( "    Processing House Keeping Packet.\n" );
				outputHK( &epmPacket );
				break;

			case GRIP_RT_ID:
				printf( "    Processing Realtime Data Packet.\n" );
				outputRT( &epmPacket );
				break;

			default:
				printf( "    discardng unrecognized EPM packet.\n" );
				break;

			}
		}

    } while( iResult > 0 ); // End loop if connection is closed or on error.
	
	// Show what caused us to exit the receiver loop.
	if ( iResult == 0 )printf("\nConnection closed by host.\n");
    else printf("\nrecv failed with error: %d\n", WSAGetLastError());

    // In this version, if the server closes the connection, we quit as well.
    closesocket(ConnectSocket);
    WSACleanup();
	printf( "Press <return>\n" );
	getchar();

	return 0;
}

