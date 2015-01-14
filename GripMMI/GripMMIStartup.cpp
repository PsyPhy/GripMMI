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

		// If both are there, we exit this dialog and allow the program to continue.
		if ( science_missing == 0 && housekeeping_missing == 0 ) {
			this->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->Close();
		}
	}

}