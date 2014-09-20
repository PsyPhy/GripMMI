// GripGroundMonitorClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\\..\\GRIPserver2\GRIPserver2\\GripPackets.h"

PCSTR EPMport = EPM_DEFAULT_PORT;
EPMTelemetryPacket epmPacket;

// Default path for packet storage is the current directory.
char *destination_directory = ".\\";

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

void outputPacket( EPMTelemetryPacket *packet, int n_bytes, const char *prefix ) {

	FILE	*fp;
	errno_t	return_code;
	size_t	items_written;
	int		bytes_written;	

	char filename[1024];
	SYSTEMTIME	systime;

	// Create a unique filename based on the current date and time.
	GetSystemTime( &systime );
	bytes_written = sprintf_s( filename, sizeof( filename ), "%s\\%s.%04d.%02d.%02d.%02d.%02d.%02d.%03d.pkt", 
		destination_directory, prefix, 
		systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds );
	if ( bytes_written < 0 ) {
			OutputDebugString( "Error in sprintf().\n" );
			exit( -1 );
	}
	return_code = fopen_s( &fp, filename, "wb" );
	if ( return_code ) {
		fMessageBox( MB_OK, "GripGroundMonitorClient", "Error opening %s for binary write.\nError code: %s", filename, return_code );
		exit( return_code );
	}
	items_written = fwrite( packet, n_bytes, 1, fp );
	if ( items_written != 1 ) {
		fMessageBox( MB_OK, "GripGroundMonitorClient", "Error writing to %s.", filename  );
		exit( return_code );
	}
	return_code = fclose( fp );
	if ( return_code ) {
		fMessageBox( MB_OK, "GripGroundMonitorClient", "Error closing %s after binary write.\nError code: %s", filename, return_code );
		exit( return_code );
	}
	printf( "%s written.\n", filename );

}

void outputHK ( EPMTelemetryPacket *packet ) {
	outputPacket( packet, hkPacketLengthInBytes, "HK" );
}
void outputRT ( EPMTelemetryPacket *packet ) {
	outputPacket( packet, rtPacketLengthInBytes, "RT" );
}

int __cdecl main(int argc, char **argv) 
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    int iResult;

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
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 104;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
		OutputDebugString( "Unable to connect to server!\n" );
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 105;
    }

#if 0
    // Send an initial buffer
    iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf)+1, 0 );
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 106;
    }

    printf("Bytes Sent: %ld\n", iResult);

    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 107;
    }
#endif 

    // Receive as long as we get packets from the server or until <ctrl-C>.
    do {

        iResult = recv(ConnectSocket, epmPacket.buffer, EPM_BUFFER_LENGTH, 0);
		if ( iResult == EPM_BUFFER_LENGTH ) {
			// If we get a full buffer of data, it probably means that we have fallen behind.
			// No packets that we expect from GRIP should use the full EPM buffer length.
			// So just skip this packet and move on to the next.
             printf("Bytes received: %4d - flushing (overrun).\n", iResult);
		}
        else if ( iResult > 0 ) {
            printf("Bytes received: %4d", iResult);
			if ( epmPacket.header.epmSyncMarker != EPM_TELEMETRY_SYNC_VALUE ) printf( " - flushing (non EPM).\n" ); 
			else printf( " TM Code 0x%04x  TM Counter %03d\n", epmPacket.header.TMIdentifier, epmPacket.header.TMCounter );
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
        else if ( iResult == 0 )printf("Connection closed\n");
        else printf("recv failed with error: %d\n", WSAGetLastError());

    } while( iResult > 0 );

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();
	printf( "Press <return>\n" );

	return 0;
}

