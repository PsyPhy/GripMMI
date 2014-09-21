// TestSimultaneousReadFromCache.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\..\GRIPserver2\GRIPserver2\GripPackets.h"
#include "..\Useful\fMessageBox.h"

EPMTelemetryPacket epmPacket;

char *input_directory = ".\\";
// Buffers to hold the path to the packet caches.
char rtPacketOutputFilePath[1024];
char hkPacketOutputFilePath[1024];

int _tmain(int argc, char *argv[])
{

	EPMTelemetryPacket *packet;
	int count = 0;

	if ( argc < 2 ) printf( "Using default input directory: %s\n", input_directory );
	else {
		input_directory = argv[1];
		printf( "Using command-line packet output directory: %s\n", input_directory );
	}

	// Create the file names that hold the packets based on today's date.
	SYSTEMTIME	systime;
	int	bytes_written;
	GetSystemTime( &systime );
	bytes_written = sprintf_s( rtPacketOutputFilePath, sizeof( rtPacketOutputFilePath ), "%s\\RT.%04d.%02d.%02d.pkt", 
		input_directory, systime.wYear, systime.wMonth, systime.wDay );
	if ( bytes_written < 0 ) {
			OutputDebugString( "Error in sprintf().\n" );
			exit( -1 );
	}
	bytes_written = sprintf_s( hkPacketOutputFilePath, sizeof( hkPacketOutputFilePath ), "%s\\HK.%04d.%02d.%02d.pkt", 
		input_directory, systime.wYear, systime.wMonth, systime.wDay );
	if ( bytes_written < 0 ) {
			OutputDebugString( "Error in sprintf().\n" );
			exit( -1 );
	}

	while ( 1 ) {

		char *filename = rtPacketOutputFilePath;
		packet = &epmPacket;
		int packets_read = 0;
		int return_code;
		FILE *fp;

		fprintf( stderr, "Openning ... " );
		fp = fopen( filename, "rb" );
		fprintf( stderr, "open" );
		if ( !fp ) {
			fMessageBox( MB_OK, "GripGroundMonitorClient", "Error opening %s for binary read.", filename );
			exit( -1 );
		}

		while ( 1 == fread( packet, rtPacketLengthInBytes, 1, fp ) ) {
			packets_read++;
			fprintf( stderr, "." );
		}
		if ( ferror( fp ) ) {
			fMessageBox( MB_OK, "GripGroundMonitorClient", "Error reading from %s.", filename  );
			exit( return_code );
		}

		fprintf( stderr, " closing ... " );
		return_code = fclose( fp );
		if ( return_code ) {
			fMessageBox( MB_OK, "GripGroundMonitorClient", "Error closing %s after binary read.\nError code: %s", filename, return_code );
			exit( return_code );
		}
		fprintf( stderr, "closed.\n" );

		fprintf( stderr, "%s Read count: %3d  Items: %d\n", filename, count++, packets_read );
		Sleep( 1000 );
	}
	

	return 0;
}

