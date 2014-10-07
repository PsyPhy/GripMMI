// ExamineGripPacketDumps.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\Useful\Useful.h"
#include "..\Useful\fMessageBox.h"
#include "..\Grip\GripPackets.h"

char *HKfilename = "C:\\GripGroundPacketStore\\grip-fm-RT.txt";

unsigned char buffer[2048];

EPMTelemetryHeaderInfo  header;
GripRealtimeDataInfo	realtime;

int rtWiresharkPacketSize = 856;
int hkWiresharkPacketSize = 280;


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

	// Pointer to the start of an EPM packet.
	EPMTelemetryPacket *epmPacket =(EPMTelemetryPacket *)(buffer + 66);
	int i, j;

	unsigned long min_time = 0;
	unsigned long max_time = 0;


	EPMTelemetryHeaderInfo header;

	fp = fopen( HKfilename, "r" );
	if ( !fp ) {
		fMessageBox( MB_OK, NULL, "Error opening %s for read.\n", HKfilename );
		exit( -1 );
	}
	for ( j = 0; j < 10; j++ ) {
	
		// Read in a packet. ReadPacketHexDump returns 0 on success and
		//  -1 on failure/end-of-file. So if not success break out of loop.
		if ( ReadPacketHexDump( fp, buffer, rtWiresharkPacketSize  ) ) break;

		// Fill the structures with the telemetry data.
		// We are reading from a Wiresharc dump, so we have to skip over 66 bytes to get to the TCP packet.
		ExtractEPMTelemetryHeaderInfo( &header, epmPacket );
		ExtractGripRealtimeDataInfo( &realtime, epmPacket );

		if ( header.coarseTime > max_time ) max_time = header.coarseTime;
		if ( min_time == 0 || header.coarseTime < min_time ) min_time = header.coarseTime;

		printf( "Packet %4d: ", packets );
		printf( "0x%08x SubS:%02d Dst:0x%02x ID:0x%04x Cnt:%6d Time:%8.3lf Blk:%2d Frm:%5d", 
			header.epmSyncMarker, header.subsystemID, header.destination, header.TMIdentifier, header.TMCounter, EPMtoSeconds( header ) - min_time,
			realtime.acquisitionID, realtime.rtPacketCount
		);
//		for ( i = 0; i < RT_SLICES_PER_PACKET; i++ ) printf( " %d:%d", realtime.dataSlice[i].poseTick, realtime.dataSlice[i].analogTick );
		printf( " Vsbl:" );
		for ( i = 0; i < RT_SLICES_PER_PACKET; i += 5 ) {
			printf( " %05x:%05x %1x", realtime.dataSlice[i].markerVisibility[0] & 0x0fffff, realtime.dataSlice[i].markerVisibility[1] & 0x0fffff, ( realtime.dataSlice[i].manipulandumVisibility ? 1 : 0 ) );
			printf( " <%4d %4d %4d>",  realtime.dataSlice[i].position[X],  realtime.dataSlice[i].position[Y], realtime.dataSlice[i].position[Z] );
			printf( " %d", realtime.dataSlice[i].acceleration[Z] );
		}
		printf( "\n" );
		// HK ID
		packets++;
	}
	fclose( fp );
	printf( "Time min: %d  max: %d  elapsed: %d (%lf hours) Avg period: %lf\n", min_time, max_time, (max_time - min_time), ((double)(max_time - min_time)) / 60.0 / 60.0, ((double) (max_time - min_time) / (double) packets ) );
	printf( "Press return.\n" );
	getchar();
	return 0;
}

