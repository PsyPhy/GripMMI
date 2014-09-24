// ExamineGripPacketDumps.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\Useful\fMessageBox.h"
#include "..\Grip\GripPackets.h"

char *HKfilename = "C:\\GripGroundPacketStore\\grip-fm-RT.txt";

unsigned char buffer[1024];

EPMTelemetryHeaderInfo  header;
GripRealtimeDataPacket	realtime;


int ReadPacketHexDump( FILE *fp, unsigned char buffer[], int n_bytes ) {

	int bytes = 0, i;
	unsigned int linecounter;
	char c;
	int items;

	for ( bytes = 0; bytes < n_bytes; ) {
		if ( 1 != fscanf( fp, "%x", &linecounter ) ) return( -1 );
		for ( i = 0; i < 16 && bytes < n_bytes ; i++, bytes++ ) {
			if ( 1 != fscanf( fp, "%x", &buffer[bytes] ) ) break;
		}
		do {
			items = fscanf( fp, "%c", &c);
		} while( items && c != '\n');
	}
	return( 0 );
}

int _tmain(int argc, _TCHAR* argv[])
{

	FILE *fp;
	int packets = 0;

	int epm = 66;
	int epmtype1 = epm+8;
	int epmtype2 = epmtype1+1;
	int grip = epm + 30;
	int user = grip + 74;
	int protocol = grip+76;
	int task = grip+78;
	int step = grip+80;

	int i;

	unsigned long min_time = 0;
	unsigned long max_time = 0;

	EPMTelemetryHeaderInfo header;

	fp = fopen( HKfilename, "r" );
	if ( !fp ) {
		fMessageBox( MB_OK, NULL, "Error opening %s for read.\n", HKfilename );
		exit( -1 );
	}
	while ( !ReadPacketHexDump( fp, buffer, 856 ) ) {

		// Fill the structures with the telemetry data.
		// We are reading from a Wiresharc dump, so we have to skip over 66 bytes to get to the TCP packet.
		ExtractEPMTelemetryHeaderInfo( &header, (EPMTelemetryPacket *) (buffer + 66) );
		ExtractGripRealtimeDataPacket( &realtime, (EPMTelemetryPacket *) (buffer + 66) );

		printf( "Packet %4d: ", packets );
		printf( "Sync: 0x%08x  ID: 0x%04x  Counter: %6d  Time:%8.3lf Block: %2d Frame: %5d Ticks: ", 
			header.epmSyncMarker, header.TMIdentifier, header.TMCounter, EPMtoSeconds( header ),
			realtime.acquisitionID, realtime.rtPacketCount
		);
		for ( i = 0; i < RT_SLICES_PER_PACKET; i += 5 ) printf( " %d", realtime.dataSlice[i].poseTick );
		printf( "\n" );
		if ( header.coarseTime > max_time ) max_time = header.coarseTime;
		if ( min_time == 0 || header.coarseTime < min_time ) min_time = header.coarseTime;
		// HK ID
		packets++;
	}
	fclose( fp );


	printf( "Time min: %d  max: %d  elapsed: %d (%lf hours) Avg period: %lf\n", min_time, max_time, (max_time - min_time), ((double)(max_time - min_time)) / 60.0 / 60.0, ((double) (max_time - min_time) / (double) packets ) );
	printf( "Press return.\n" );
	getchar();
	return 0;
}

