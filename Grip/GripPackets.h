//
// Packet definitions for realtime data from Grip.
//
#pragma once

// The port number used to access EPM servers.
// EPM-OHB-SP-0005 says:
//  The Port number for all EPM LAN connections is 2345.

#define EPM_DEFAULT_PORT "2345"

// Per EPM-OHB-SP-0005, packets shall not exceed 1412 octets.
#define EPM_BUFFER_LENGTH	1412	

// Telemetry packets have the following sync marker.
#define EPM_TELEMETRY_SYNC_VALUE	0xFFDB544D
#define GRIP_HK_ID	0x0301
#define GRIP_RT_ID	0x1001

#define EPMtoSeconds( header ) ((long double)header.coarseTime + ((long double) header.fineTime / 10000.0))
typedef struct {

	unsigned long  epmSyncMarker;
	unsigned char  subsystemMode;
	unsigned char  subsystemID;
	unsigned char  destination;
	unsigned char  subsystemUnitID;
	unsigned short TMIdentifier;
	unsigned short TMCounter;
	unsigned char  model;
	unsigned char  taskID;
	unsigned short subsystemUnitVersion;
	unsigned long  coarseTime;
	unsigned short fineTime;
	unsigned char  timerStatus;
	unsigned char  experimentMode;
	unsigned short checksumIndicator;
	unsigned char  receiverSubsystemID;
	unsigned char  receiverSubsystemUnitID;
	unsigned short numberOfWords;

} EPMTelemetryHeaderInfo;

typedef struct {
	// DATA_MANIP_POSE
	unsigned long	poseTick;
	short			position[3];
	float			quaternion[4];
	unsigned long	markerVisbility[2];  // One for each coda;
	unsigned char	visibility;
	// DATA_IOC_FTG
	unsigned long	analogTick;
	short			ft[2][6];
	long			acc[3];
} ManipulandumPacket;
 
#define RT_SLICES_PER_PACKET 10
typedef struct {
	unsigned long acquisitionID;
	unsigned long rtPacketCount;
	ManipulandumPacket dataSlice[RT_SLICES_PER_PACKET];
} GripRealtimeDataPacket;

typedef union {
	// A buffer containing the entire packet.
	char					buffer[EPM_BUFFER_LENGTH];
	// Allows easy acces to the header elements by name.
	EPMTelemetryHeaderInfo	byte_reversed_header;
} EPMTelemetryPacket;

// Define a static packet header that is representative of a housekeeping packet.
// We don't try to simulate all the details, so most of the parameters are set to zero.
// The TM Identifier is 0x0301 for DATA_BULK_HK per DEX-ICD-00383-QS.
// The total number of words is 114 / 2 = 57 for the GRIP packet, 15 for the EPM header and 1 for the checksum.
static EPMTelemetryHeaderInfo hkHeader = { EPM_TELEMETRY_SYNC_VALUE, 0, 0, 0, 0, GRIP_HK_ID, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static int hkPacketLengthInBytes = 146;

// Do the same for a realtime data packet.
// The TM Identifier is 0x1001 for DATA_RT_SCIENCE per DEX-ICD-00383-QS.
// The total number of words is 758 / 2 = 379 for the GRIP packet, 15 for the EPM header and 1 for the checksum.
static EPMTelemetryHeaderInfo rtHeader = { EPM_TELEMETRY_SYNC_VALUE, 0, 0, 0, 0, GRIP_RT_ID, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static int rtPacketLengthInBytes = 790;

#ifdef __cplusplus
extern "C" {
#endif

void ExtractEPMTelemetryHeaderInfo ( EPMTelemetryHeaderInfo *header, const EPMTelemetryPacket *epm_packet  );
void InsertEPMTelemetryHeaderInfo ( EPMTelemetryPacket *epm_packet,  const EPMTelemetryHeaderInfo *header  );
void ExtractGripRealtimeDataPacket( GripRealtimeDataPacket *realtime_packet, const EPMTelemetryPacket *epm_packet );

#ifdef __cplusplus
}
#endif 
 