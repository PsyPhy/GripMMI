///
/// Module:	GripMMI
/// 
///	Author:					J. McIntyre, PsyPhy Consulting
/// Initial release:		18 December 2014
/// Modification History:	see https://github.com/PsyPhy/GripMMI
///
/// Copyright (c) 2014, 2015 PsyPhy Consulting
///

// Methods to wait for the intial packets to arrive before proceeding.
// GripMMI pops up an initial window to show the wait state and shows where
//  it is expecting to find the packet buffer caches.

#include "StdAfx.h"

#include "..\Grip\GripPackets.h"
#include "..\Grip\DexAnalogMixin.h"
#include "GripMMIGlobals.h"
#include "GripMMIStartup.h"

#include <io.h>

namespace GripMMI {

	void GripMMIStartup::DisplayCachePaths( void ) {
		// Create and display paths to the packet caches.
		char rtCacheFilename[1024];
		char hkCacheFilename[1024];
		CreateGripPacketCacheFilename( rtCacheFilename, sizeof( rtCacheFilename ), GRIP_RT_SCIENCE_PACKET, packetBufferPathRoot );
		rtCacheFilenameText->Text = gcnew String( rtCacheFilename );
		CreateGripPacketCacheFilename( hkCacheFilename, sizeof( hkCacheFilename ), GRIP_HK_BULK_PACKET, packetBufferPathRoot );
		hkCacheFilenameText->Text = gcnew String( hkCacheFilename );
	}


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

		char rtCacheFilename[1024];
		CreateGripPacketCacheFilename( rtCacheFilename, sizeof( rtCacheFilename ), GRIP_HK_BULK_PACKET, packetBufferPathRoot );
		int science_missing = _access( rtCacheFilename, 00 );
		char hkCacheFilename[1024];
		CreateGripPacketCacheFilename( hkCacheFilename, sizeof( hkCacheFilename ), GRIP_HK_BULK_PACKET, packetBufferPathRoot );
		int housekeeping_missing = _access( hkCacheFilename, 00 );

		// If both are there, we exit this dialog with OK status, which will allow the program to continue.
		// If the user closes the window by the cancel button or close button, exit status will be Cancel 
		//  and the application will exit.
		if ( science_missing == 0 && housekeeping_missing == 0 ) {
			this->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->Close();
		}
	}

}