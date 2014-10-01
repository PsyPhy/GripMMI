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
#define EPM_TRANSFER_FRAME_HEADER_LENGTH	12
#define EPM_TELEMETRY_HEADER_LENGTH			30

// Definitions for Transfer Frame headers.
#define EPM_TRANSFER_FRAME_SYNC_VALUE	0xAA49DBFF
#define TRANSFER_FRAME_CONNECT			0x0001
#define TRANSFER_FRAME_ALIVE			0x0002
#define TRANSFER_FRAME_TELECOMMAND		0x1154
#define TRANSFER_FRAME_TELEMETRY		0x1153

#define GRIP_MMI_SOFTWARE_UNIT_ID		0x43
#define GRIP_SUBSYSTEM_ID				0x21

// These constants help make it clear in initialization lists
//  when we are just filling a spare slot in a structure or
//  when we are initializing a field for which the value is not
//  yet known and will be filled in later.
#define SPARE	0
#define UNKNOWN	0

// Definitions for Telemetry Packet headers.
#define EPM_TELEMETRY_SYNC_VALUE		0xFFDB544D
#define GRIP_HK_ID	0x0301
#define GRIP_RT_ID	0x1001

// Compute the time in seconds.
#define EPMtoSeconds( header ) ((long double)header.coarseTime + ((long double) header.fineTime / 10000.0))

typedef struct {
	unsigned long	epmLanSyncMarker;
	unsigned char	spare1;
	unsigned char	softwareUnitID;
	unsigned short	packetType;
	unsigned short	spare2;
	unsigned short	numberOfWords;
} EPMTransferFrameHeaderInfo;

typedef struct {

	EPMTransferFrameHeaderInfo	transferFrameInfo;

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
	unsigned long	markerVisibility[2];  // One for each coda;
	unsigned char	manipulandumVisibility;
	// DATA_IOC_FTG
	unsigned long	analogTick;
	float			ft[2][6];
	float			acceleration[3];
} ManipulandumPacket;
 
#define RT_SLICES_PER_PACKET 10
typedef struct {
	unsigned long acquisitionID;
	unsigned long rtPacketCount;
	ManipulandumPacket dataSlice[RT_SLICES_PER_PACKET];
} GripRealtimeDataInfo;

typedef struct {

#if 0

	// ExtractGripHealthAndStatusInfo() does not yet fill in the entire structure.
	// These values are hidden inside the #if 0 so that no one tries to use them.
	// When ExtractGripHealthAndStatusInfo() gets update, these items can be exposed again.
	unsigned short	currentMode;
	unsigned short	nextMode;
	unsigned short	timerStatus;
	unsigned short	correctiveAction;
	unsigned short	fileTransferStatus;
	// Various temperature and voltage readings.
	// We don't use them, so I don't name them separately.
	short	temperature[10];
	short	voltage[8];
	unsigned short	selftest;
	float	rxDataRate;
	float	txDataRate;
	unsigned short	fanStatus;
	unsigned short	epmInteraceStatusEnum;
	
	unsigned long	unexplained;

	unsigned short	smokeDetectorStatus;
	unsigned short  OCD;

#endif

	unsigned short	horizontalTargetFeedback;
	unsigned short	verticalTargetFeedback;
	unsigned char	toneFeedback;
	unsigned char	cradleDetectors;

	unsigned short	user;
	unsigned short	protocol;
	unsigned short	task;
	unsigned short	step;

	unsigned short	scriptEngineStatusEnum;
	unsigned short	iochannelStatusEnum;
	unsigned short	motionTrackerStatusEnum;
	unsigned short	crewCameraStatusEnum;

	unsigned short	crewCameraRate; // fps

	unsigned short	runningBits;	// Bit 0: shell command  Bit 1: system acquiring
	unsigned short	cpuUsage;		// percent
	unsigned short	memoryUsage;	// percent

	unsigned long	freeDiskSpaceC;
	unsigned long	freeDiskSpaceD;
	unsigned long	freeDiskSpaceE;

	unsigned short	crc;

} GripHealthAndStatusInfo;

typedef union {
	// A buffer containing the entire packet.
	char buffer[EPM_BUFFER_LENGTH];
	// Allows easy acces to the header elements by name.
	struct {
		char rawTransferFrameHeader[EPM_TRANSFER_FRAME_HEADER_LENGTH];
		char rawTelemetryHeader[EPM_TELEMETRY_HEADER_LENGTH];
		char rawData[EPM_BUFFER_LENGTH - (EPM_TRANSFER_FRAME_HEADER_LENGTH + EPM_TELEMETRY_HEADER_LENGTH)];
	} sections;
} EPMTelemetryPacket; 

// Define a static lan packet for sending a connect command from the GRIP-MMI to EPM.
static EPMTransferFrameHeaderInfo connectPacket = { EPM_TRANSFER_FRAME_SYNC_VALUE, SPARE, GRIP_MMI_SOFTWARE_UNIT_ID, TRANSFER_FRAME_CONNECT, SPARE, 6 };
static int connectPacketLengthInBytes = 12;
static int connectPacketLengthInWords = 6;

static EPMTransferFrameHeaderInfo alivePacket = { EPM_TRANSFER_FRAME_SYNC_VALUE, SPARE, GRIP_MMI_SOFTWARE_UNIT_ID, TRANSFER_FRAME_ALIVE, SPARE, 6 };
static int alivePacketLengthInBytes = 12;
static int alivePacketLengthInWords = 6;

// Define a static packet header that is representative of a housekeeping packet.
// We don't try to simulate all the details, so most of the parameters are set to zero.
// The TM Identifier is 0x0301 for DATA_BULK_HK per DEX-ICD-00383-QS.
// The total number of words is 114 / 2 = 57 for the GRIP packet, 6 for the Transfer Frame header, 
//  15 for the Telemetry header and 1 for the checksum.
static EPMTelemetryHeaderInfo hkHeader = { 
	EPM_TRANSFER_FRAME_SYNC_VALUE, SPARE, GRIP_MMI_SOFTWARE_UNIT_ID, TRANSFER_FRAME_TELEMETRY, SPARE, 158,
	EPM_TELEMETRY_SYNC_VALUE, 0, GRIP_SUBSYSTEM_ID, 0, 0, GRIP_HK_ID, UNKNOWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static int hkPacketLengthInBytes = 158;

// Define a static packet header that is representative of a realtime data packet.
// Not all of the members are properly filled. Just the ones important for the GripMMMI.
// The TM Identifier is 0x1001 for DATA_RT_SCIENCE per DEX-ICD-00383-QS.
// The total number of words is 758 / 2 = 379 for the GRIP packet, 6 for the Transfer Frame header,
//  15 for the EPM header and 1 for the checksum.
static EPMTelemetryHeaderInfo rtHeader = { 
	EPM_TRANSFER_FRAME_SYNC_VALUE, SPARE, GRIP_MMI_SOFTWARE_UNIT_ID, TRANSFER_FRAME_TELEMETRY, SPARE, 796,
	EPM_TELEMETRY_SYNC_VALUE, 0, GRIP_SUBSYSTEM_ID, 0, 0, GRIP_RT_ID, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static int rtPacketLengthInBytes = 796;

typedef enum { GRIP_RT_SCIENCE_PACKET, GRIP_HK_BULK_PACKET, GRIP_UNKNOWN_PACKET } GripPacketType;

#ifdef __cplusplus
extern "C" {
#endif

int  InsertEPMTransferFrameHeaderInfo ( EPMTelemetryPacket *epm_packet, const EPMTransferFrameHeaderInfo *header  );
void ExtractEPMTransferFrameHeaderInfo ( EPMTransferFrameHeaderInfo *header, const EPMTelemetryPacket *epm_packet );
void ExtractEPMTelemetryHeaderInfo ( EPMTelemetryHeaderInfo *header, const EPMTelemetryPacket *epm_packet  );
int  InsertEPMTelemetryHeaderInfo ( EPMTelemetryPacket *epm_packet,  const EPMTelemetryHeaderInfo *header  );
void ExtractGripRealtimeDataInfo( GripRealtimeDataInfo *realtime_packet, const EPMTelemetryPacket *epm_packet );
void ExtractGripHealthAndStatusInfo( GripHealthAndStatusInfo *health_packet, const EPMTelemetryPacket *epm_packet );

void CreateGripPacketCacheFilename( char *filename, int max_characters, const GripPacketType type, const char *root );

#ifdef __cplusplus
}
#endif 
 