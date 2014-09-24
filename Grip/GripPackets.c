//
// Packet definitions for realtime data from Grip.
//

#include "GripPackets.h"

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

unsigned  short extract_short( const unsigned char bytes[2] ) {
	union {
		unsigned short value;
		unsigned char  byte[2];
	} out;
	out.byte[0] = bytes[1];
	out.byte[1] = bytes[0];
	return( out.value );
}

unsigned long extract_long( const unsigned char bytes[4] ) {
	union {
		unsigned long value;
		unsigned char  byte[4];
	} out;
	out.byte[0] = bytes[3];
	out.byte[1] = bytes[2];
	out.byte[2] = bytes[1];
	out.byte[3] = bytes[0];
	return( out.value );
}

/***********************************************************************************/

void InsertEPMTelemetryHeaderInfo ( EPMTelemetryPacket *epm_packet, const EPMTelemetryHeaderInfo *header  ) {

	// Fill a structure with the header values from an EPM TCP packet.
	// The bytes are in ESA/EPM order in the TCP packet, and need to be reversed for Windows.
	unsigned char *ptr = ((unsigned char *) epm_packet); 
	*((long *)ptr)= swapbytes_long( header->epmSyncMarker ); ptr += sizeof( unsigned long );
	*ptr = header->subsystemMode; ptr++;
	*ptr = header->subsystemID; ptr++;
	*ptr = header->destination; ptr++;
	*ptr = header->subsystemUnitID; ptr++;
	*((unsigned short *)ptr) = swapbytes_short( header->TMIdentifier ); ptr += sizeof( short );
	*((unsigned short *)ptr) = swapbytes_short( header->TMCounter ); ptr += sizeof( short );
	*ptr = header->model; ptr++;
	*ptr = header->taskID; ptr++;
	*((unsigned short *)ptr) = swapbytes_short( header->subsystemUnitVersion ); ptr += sizeof( short );
	*((unsigned long  *)ptr) = swapbytes_long( header->coarseTime ); ptr += sizeof( long );
	*((unsigned short *)ptr) = swapbytes_short( header->fineTime ); ptr += sizeof( short );
	*ptr = header->timerStatus; ptr++;
	*ptr = header->experimentMode; ptr++;
	*((unsigned short *)ptr) = swapbytes_short( header->checksumIndicator ); ptr += sizeof( short );
	*ptr = header->receiverSubsystemID; ptr++;
	*ptr = header->receiverSubsystemUnitID; ptr++;
	*((unsigned short *)ptr) = swapbytes_short( header->numberOfWords ); ptr += sizeof( short );
}

void ExtractEPMTelemetryHeaderInfo ( EPMTelemetryHeaderInfo *header, const EPMTelemetryPacket *epm_packet  ) {

	// Fill a structure with the header values from an EPM TCP packet.
	// The bytes are in ESA/EPM order in the TCP packet, and need to be reversed for Windows.
	unsigned char *ptr = ((unsigned char *) epm_packet); 
	header->epmSyncMarker = extract_long( ptr ); ptr += sizeof( unsigned long );
	header->subsystemMode = *ptr; ptr++;
	header->subsystemID = *ptr; ptr++;
	header->destination= *ptr; ptr++;
	header->subsystemUnitID = *ptr; ptr++;
	header->TMIdentifier = extract_short( ptr ); ptr += sizeof( short );
	header->TMCounter = extract_short( ptr ); ptr += sizeof( short );
	header->model = *ptr; ptr++;
	header->taskID = *ptr; ptr++;
	header->subsystemUnitVersion = extract_short( ptr ); ptr += sizeof( short );
	header->coarseTime = extract_long( ptr ); ptr += sizeof( long );
	header->fineTime = extract_short( ptr ); ptr += sizeof( short );
	header->timerStatus = *ptr; ptr++;
	header->experimentMode = *ptr; ptr++;
	header->checksumIndicator = extract_short( ptr ); ptr += sizeof( short );
	header->receiverSubsystemID = *ptr; ptr++;
	header->receiverSubsystemUnitID = *ptr; ptr++;
	header->numberOfWords = extract_short( ptr ); ptr += sizeof( short );

}

// Extract a real-time data packet from an EPM packet.
void ExtractGripRealtimeDataPacket( GripRealtimeDataPacket *realtime_packet, const EPMTelemetryPacket *epm_packet ) {
	unsigned char *ptr = ((unsigned char *) epm_packet);
	int slice;
	// Skip over the EPM header.
	ptr += 30;
	// Get the acquisition ID and packet count for that acquisition.
	realtime_packet->acquisitionID = extract_long( ptr ); ptr += sizeof( unsigned long );
	realtime_packet->rtPacketCount = extract_long( ptr ); ptr += sizeof( unsigned long );
	for ( slice = 0; slice < RT_SLICES_PER_PACKET; slice++ ) {
		// Get the manipulandum pose data. 
		// For the moment, just get the ticks and jump to the next entry.
		realtime_packet->dataSlice[slice].poseTick = extract_long( ptr ); ptr += 35;
		realtime_packet->dataSlice[slice].analogTick = extract_long( ptr ); ptr += 40;
	}
}
