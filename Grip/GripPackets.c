//
// Packet definitions for realtime data from Grip.
//

#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>

#include "..\Useful\fMessageBox.h"
#include "..\Useful\fOutputDebugString.h"
#include "..\Useful\Useful.h"

#include "GripPackets.h"

// Routines to change the byte order in various data types.
// These are useful when inserting or extracting data from an EPM packet.

unsigned short swapbytes_short( unsigned short input ) {
	union {
		unsigned short value;
		unsigned char  byte[2];
	} in, out;
	in.value = input;
	out.byte[0] = in.byte[1];
	out.byte[1] = in.byte[0];
	return( out.value );
}

unsigned long swapbytes_long( unsigned long input ) {
	union {
		unsigned long value;
		unsigned char  byte[4];
	} in, out;
	in.value = input;
	out.byte[0] = in.byte[3];
	out.byte[1] = in.byte[2];
	out.byte[2] = in.byte[1];
	out.byte[3] = in.byte[0];
	return( out.value );
}

// These routines are used to insert or extract various data types 
//  from a byte buffer without changing the byte order.
short extract_short( const unsigned char bytes[2] ) {
	union {
		unsigned short value;
		unsigned char  byte[2];
	} out;
	out.byte[0] = bytes[0];
	out.byte[1] = bytes[1];
	return( out.value );
}

// These routines are used to insert or extract various data types 
//  in reverse byte order from a byte buffer.
short extract_reversed_short( const unsigned char bytes[2] ) {
	union {
		short			value;
		unsigned char	byte[2];
	} out;
	out.byte[0] = bytes[1];
	out.byte[1] = bytes[0];
	return( out.value );
}

long extract_reversed_long( const unsigned char bytes[4] ) {
	union {
		long			value;
		unsigned char	byte[4];
	} out;
	out.byte[0] = bytes[3];
	out.byte[1] = bytes[2];
	out.byte[2] = bytes[1];
	out.byte[3] = bytes[0];
	return( out.value );
}

float extract_reversed_float( const unsigned char bytes[4] ) {
	union {
		float			value;
		unsigned char	byte[4];
	} out;
	out.byte[0] = bytes[3];
	out.byte[1] = bytes[2];
	out.byte[2] = bytes[1];
	out.byte[3] = bytes[0];
	return( out.value );
}
// These macros facilate stepping through a buffer with a pointer and make the code cleaner.
// On each use of the macro, the pointer is incremented by the number of bytes corresponding to the data type.
// Use of the macros reduces the chance of error due to mismatch between data type and increment of the pointer.
// Note the comma, rather than semicolon, between statements. This make it OK to put it in a single-line 
// 'if' or 'for' statement, i.e. "for ( i = X; i <= Z; i++ ) position[i] = ExtractShort( ptr );"
#define ExtractShort( ptr ) extract_short( ptr ), ptr += sizeof( short ) 
#define ExtractReversedShort( ptr ) extract_reversed_short( ptr ), ptr += sizeof( short ) 
#define ExtractReversedLong( ptr ) extract_reversed_long( ptr ), ptr += sizeof( long ) 
#define ExtractReversedFloat( ptr ) extract_reversed_float( ptr ), ptr += sizeof( float ) 
#define ExtractChar( ptr ) (*ptr++)

/***********************************************************************************/

// Fill a structure with the header values from an EPM Transfer Frame packet.
// The bytes are in ESA/EPM order in the packet, and need to be reversed for Windows.
// Returns the number of bytes inserted into the buffer.
int InsertEPMTransferFrameHeaderInfo ( EPMTelemetryPacket *epm_packet, const EPMTransferFrameHeaderInfo *header  ) {
	unsigned char *ptr = ((unsigned char *) epm_packet); 
	unsigned int  bytes_inserted = 0;
	*((long *)ptr)= swapbytes_long( header->epmLanSyncMarker ); ptr += sizeof( unsigned long ); bytes_inserted += sizeof( unsigned long);
	*ptr =  header->spare1; ptr++; bytes_inserted++;
	*ptr = header->softwareUnitID; ptr++; bytes_inserted++;
	*((unsigned short *)ptr) = swapbytes_short( header->packetType ); ptr += sizeof( unsigned short ); bytes_inserted += sizeof( unsigned short);
	*((unsigned short *)ptr) = swapbytes_short( header->spare2 ); ptr += sizeof( unsigned short );  bytes_inserted += sizeof( unsigned short);
	*((unsigned short *)ptr) = swapbytes_short( header->numberOfWords ); ptr += sizeof( unsigned short );  bytes_inserted += sizeof( unsigned short);
	return( bytes_inserted );
}

void ExtractEPMTransferFrameHeaderInfo ( EPMTransferFrameHeaderInfo *header, const EPMTelemetryPacket *epm_packet  ) {
	unsigned char *ptr = ((unsigned char *) epm_packet); 
	header->epmLanSyncMarker = ExtractReversedLong( ptr );
	header->spare1 = ExtractChar( ptr );
	header->softwareUnitID = ExtractChar( ptr );
	header->packetType = ExtractReversedShort( ptr );
	header->spare2 = ExtractReversedShort( ptr );
	header->numberOfWords = ExtractReversedShort( ptr );
}

// Insert Telemetry header info into a buffer in byte-reversed order.
// Returns the number of bytes inserted.
int InsertEPMTelemetryHeaderInfo ( EPMTelemetryPacket *epm_packet, const EPMTelemetryHeaderInfo *header  ) {
	// Fill a structure with the header values from an EPM TCP packet.
	// The bytes are in ESA/EPM order in the TCP packet, and need to be reversed for Windows.
	unsigned char *ptr = ((unsigned char *) epm_packet); 
	unsigned int  bytes_inserted = 0;

	bytes_inserted = InsertEPMTransferFrameHeaderInfo ( epm_packet, &header->transferFrameInfo );
	ptr += bytes_inserted;

	*((long *)ptr)= swapbytes_long( header->epmSyncMarker ); ptr += sizeof( unsigned long ); bytes_inserted += sizeof( unsigned long );
	*ptr = header->subsystemMode; ptr++; bytes_inserted++;
	*ptr = header->subsystemID; ptr++; bytes_inserted++;
	*ptr = header->destination; ptr++; bytes_inserted++;
	*ptr = header->subsystemUnitID; ptr++; bytes_inserted++;
	*((unsigned short *)ptr) = swapbytes_short( header->TMIdentifier ); ptr += sizeof( unsigned short );  bytes_inserted += sizeof( unsigned short );
	*((unsigned short *)ptr) = swapbytes_short( header->TMCounter ); ptr += sizeof( unsigned short ); bytes_inserted += sizeof( unsigned short );
	*ptr = header->model; ptr++; bytes_inserted++;
	*ptr = header->taskID; ptr++; bytes_inserted++;
	*((unsigned short *)ptr) = swapbytes_short( header->subsystemUnitVersion ); ptr += sizeof( unsigned short ); bytes_inserted += sizeof( unsigned short );
	*((unsigned long  *)ptr) = swapbytes_long( header->coarseTime ); ptr += sizeof( unsigned long ); bytes_inserted += sizeof( unsigned long );
	*((unsigned short *)ptr) = swapbytes_short( header->fineTime ); ptr += sizeof( unsigned short ); bytes_inserted += sizeof( unsigned short );
	*ptr = header->timerStatus; ptr++; bytes_inserted++;
	*ptr = header->experimentMode; ptr++; bytes_inserted++;
	*((unsigned short *)ptr) = swapbytes_short( header->checksumIndicator ); ptr += sizeof( unsigned short ); bytes_inserted += sizeof( unsigned short );
	*ptr = header->receiverSubsystemID; ptr++; bytes_inserted++;
	*ptr = header->receiverSubsystemUnitID; ptr++; bytes_inserted++;
	*((unsigned short *)ptr) = swapbytes_short( header->numberOfWords ); ptr += sizeof( unsigned short ); bytes_inserted += sizeof( unsigned short );
	return( bytes_inserted );
}

void ExtractEPMTelemetryHeaderInfo ( EPMTelemetryHeaderInfo *header, const EPMTelemetryPacket *epm_packet  ) {
	// Fill a structure with the header values from an EPM TCP packet.
	// The bytes are in ESA/EPM order in the TCP packet, and need to be reversed for Windows.
	unsigned char *ptr = ((unsigned char *) epm_packet); 

	header->transferFrameInfo.epmLanSyncMarker = ExtractReversedLong( ptr );
	header->transferFrameInfo.spare1 = ExtractChar( ptr );
	header->transferFrameInfo.softwareUnitID = ExtractChar( ptr );
	header->transferFrameInfo.packetType = ExtractReversedShort( ptr );
	header->transferFrameInfo.spare2 = ExtractReversedShort( ptr );
	header->transferFrameInfo.numberOfWords = ExtractReversedShort( ptr );

	header->epmSyncMarker = ExtractReversedLong( ptr );
	header->subsystemMode = ExtractChar( ptr );
	header->subsystemID = ExtractChar( ptr );
	header->destination= ExtractChar( ptr );
	header->subsystemUnitID = ExtractChar( ptr );
	header->TMIdentifier = ExtractReversedShort( ptr );
	header->TMCounter = ExtractReversedShort( ptr );
	header->model = ExtractChar( ptr );
	header->taskID = ExtractChar( ptr );
	header->subsystemUnitVersion = ExtractReversedShort( ptr );
	header->coarseTime = ExtractReversedLong( ptr );
	header->fineTime = ExtractReversedShort( ptr );
	header->timerStatus = ExtractChar( ptr );
	header->experimentMode = *ptr; ptr++;
	header->checksumIndicator = ExtractReversedShort( ptr );
	header->receiverSubsystemID = ExtractChar( ptr );
	header->receiverSubsystemUnitID = ExtractChar( ptr );
	header->numberOfWords = ExtractReversedShort( ptr ); 
}

// Extract a real-time science data packet from an EPM packet.
void ExtractGripRealtimeDataInfo( GripRealtimeDataInfo *realtime_packet, const EPMTelemetryPacket *epm_packet ) {
	const char *ptr;
	int slice;
	int sensor;
	int i;
	short value;
	long lvalue;

	// Point to the actual data in the packet.
	ptr = epm_packet->sections.rawData;
	// Get the acquisition ID and packet count for that acquisition.
	realtime_packet->acquisitionID = ExtractReversedLong( ptr );
	realtime_packet->rtPacketCount = ExtractReversedLong( ptr );
	fOutputDebugString( "%d Acc raw: ", realtime_packet->rtPacketCount );
	for ( slice = 0; slice < RT_SLICES_PER_PACKET; slice++ ) {
		// Get the manipulandum pose data. 
		realtime_packet->dataSlice[slice].poseTick = ExtractReversedLong( ptr );
		for ( i = X; i <= Z; i++ ) realtime_packet->dataSlice[slice].position[i] = (double) ExtractReversedShort( ptr );
		for ( i = X; i <= M; i++ ) realtime_packet->dataSlice[slice].quaternion[i] = ExtractReversedFloat( ptr );
		for ( i = 0; i < 2; i++ ) realtime_packet->dataSlice[slice].markerVisibility[i] = ExtractReversedLong( ptr );
		realtime_packet->dataSlice[slice].manipulandumVisibility = ExtractChar( ptr );
		// Get the analog data.
		realtime_packet->dataSlice[slice].analogTick = ExtractReversedLong( ptr );
		fOutputDebugString( "  %6d", realtime_packet->dataSlice[slice].analogTick );
		for ( sensor = 0; sensor < 2; sensor++ ) {
			for ( i = X; i <=Z; i++ ) {
				value = ExtractReversedShort( ptr );
				realtime_packet->dataSlice[slice].ft[sensor].force[i] = (double) value / 100.0;
			}
			for ( i = X; i <=Z; i++ ) {
				value = ExtractReversedShort( ptr );
				realtime_packet->dataSlice[slice].ft[sensor].torque[i] = (double) value / 1000.0;
			}
		}
		for ( i = X; i <= Z; i++ ) {
			lvalue = ExtractReversedLong( ptr );
			realtime_packet->dataSlice[slice].acceleration[i] = ((double) lvalue) / 1000.0 / 9.8;
		}
	}
	fOutputDebugString( "\n" );

}

// Extract a Grip housekeeping packet from an EPM packet.
// Careful! It is not complete. Not all values are filled.
void ExtractGripHealthAndStatusInfo( GripHealthAndStatusInfo *health_packet, const EPMTelemetryPacket *epm_packet ) {

	const char *ptr;

	// Point to the actual data in the packet.
	ptr = epm_packet->sections.rawData;

	// Skip to the script information.
	ptr += 76;

	health_packet->horizontalTargetFeedback = ExtractReversedShort( ptr );
	health_packet->verticalTargetFeedback = ExtractReversedShort( ptr );

	health_packet->toneFeedback = ExtractChar( ptr );
	health_packet->cradleDetectors = ExtractChar( ptr );

	health_packet->user = ExtractReversedShort( ptr );
	health_packet->protocol = ExtractReversedShort( ptr );
	health_packet->task = ExtractReversedShort( ptr );
	health_packet->step = ExtractReversedShort( ptr );

	health_packet->scriptEngineStatusEnum = ExtractReversedShort( ptr );
	health_packet->iochannelStatusEnum = ExtractReversedShort( ptr );
	health_packet->motionTrackerStatusEnum = ExtractReversedShort( ptr );
	health_packet->crewCameraStatusEnum = ExtractReversedShort( ptr );

	health_packet->crewCameraRate = ExtractReversedShort( ptr );

	health_packet->runningBits = ExtractReversedShort( ptr );
	health_packet->cpuUsage = ExtractReversedShort( ptr );
	health_packet->memoryUsage = ExtractReversedShort( ptr );

	health_packet->freeDiskSpaceC = ExtractReversedLong( ptr );
	health_packet->freeDiskSpaceD = ExtractReversedLong( ptr );
	health_packet->freeDiskSpaceE = ExtractReversedLong( ptr );

	health_packet->crc = ExtractReversedShort( ptr );
	

}

void CreateGripPacketCacheFilename( char *filename, int max_characters, const GripPacketType type, const char *root ) {
		
	// Create the file names that hold the packets according to packet type.
	int	bytes_written;

	switch ( type ) {

	case GRIP_RT_SCIENCE_PACKET:
		bytes_written = sprintf( filename, "%s.rt.gpk", root );
		break;
	case GRIP_HK_BULK_PACKET:
		bytes_written = sprintf( filename, "%s.hk.gpk", root );
		break;
	default:
		bytes_written = sprintf( filename, "%s.any.gpk", root );
		break;

	}
	if ( bytes_written < 0 ) {
			fMessageBox( MB_OK, "Grip", "Error in sprintf()." );
			exit( -1 );
	}

}

