///
/// Module:	GripMMI
/// 
///	Author:					J. McIntyre, PsyPhy Consulting
/// Initial release:		18 December 2014
/// Modification History:	see https://github.com/frenchjam/GripGroundMonitorClient
///
/// Copyright (c) 2014, 2015 PsyPhy Consulting
///

/// Methods to wait for the intial packets to arrive before proceeding.

#include "StdAfx.h"

#include "..\Grip\GripPackets.h"
#include "..\Grip\DexAnalogMixin.h"
#include "GripMMIStartup.h"
#include "GripMMIGlobals.h"

#include <io.h>

namespace GripMMI {

	void GripMMIStartup::CreateRefreshTimer( int interval ) {
		timer = gcnew Timer;
		timer->Interval = interval;
		timer->Tick += gcnew EventHandler( this, &GripMMI::GripMMIStartup::OnTimerElapsed );
	}
	void GripMMIStartup::StartRefreshTimer( void ) {
		timer->Start();
	}
	void GripMMIStartup::StopRefreshTimer( void ) {
		timer->Stop();
	}

	void GripMMIStartup::OnTimerElapsed( System::Object^ source, System::EventArgs ^ e ) {

		char filename[1024];

		// Create paths to the packet caches and check if the cache files are there.
		CreateGripPacketCacheFilename( filename, sizeof( filename ), GRIP_RT_SCIENCE_PACKET, packetBufferPathRoot );
		int science_missing = _access( filename, 00 );
		CreateGripPacketCacheFilename( filename, sizeof( filename ), GRIP_HK_BULK_PACKET, packetBufferPathRoot );
		int housekeeping_missing = _access( filename, 00 );

		// If both are there, we exit this dialog with OK status, which will allow the program to continue.
		// If the user closes the window by the cancel button or close button, exit status will be Cancel 
		//  and the application will exit.
		if ( science_missing == 0 && housekeeping_missing == 0 ) {
			this->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->Close();
		}
	}

}