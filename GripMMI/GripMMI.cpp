///
/// Module:	GripMMI
/// 
///	Author:					J. McIntyre, PsyPhy Consulting
/// Initial release:		18 December 2014
/// Modification History:	see https://github.com/PsyPhy/GripMMI
///
/// Copyright (c) 2014, 2015 PsyPhy Consulting
///

/// GripMMI graphical application.
/// Packets from Grip are placed at that location by the GripGroundMonitorClient module, which is 
///  a separate process that talks to the EPM CLWS via tcp/ip to retrieve the packets.
/// This module monitors the defined data directory for the presence of GRIP realtime data packets
///  and graphs the data on the screen. It also shows the current step and screens seen by the 
///  subject, based on a local copy of the scripts and step info from the GRIP housekeeping packets.

#include "stdafx.h"

#include "GripMMIAbout.h"
#include "GripMMIStartup.h"
#include "GripMMIFullStep.h"
#include "GripMMIDesktop.h"

using namespace GripMMI;

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
	// Default locations for packet buffer file and the script tree.
	String^ scriptRoot = "scripts\\";
	String^ packetRoot = "GripPackets";

	// Enabling Windows XP visual effects before any controls are created
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 

	// Parse the command line arguments.
	if ( args->Length > 0 ) packetRoot = args[0];	// Where to look for packets written by GripGroundMonitorClient.exe
	if ( args->Length > 1 ) scriptRoot = args[1];	// Where to find the script library.
	if ( args->Length > 2 ) TimebaseOffset = Convert::ToInt32(args[2]); // Correct the time base. Default is 16 to correct to UTC. 0 is GPS time.
	
	// First, a lot of code to convert the Strings that we get from the command line
	// to the (char *) values that the legacy script crawler code needs.

	pin_ptr<const wchar_t> pinchars = PtrToStringChars( scriptRoot );
	size_t converted_chars;
	size_t  size_in_bytes;
	errno_t err = 0;

	// Create a classical character array with the path to the scriptDirectory.
	converted_chars = 0;
	size_in_bytes = ((scriptRoot->Length + 1) * 2);
	err = wcstombs_s(&converted_chars, 
			scriptDirectory, sizeof( scriptDirectory ),
			pinchars, _TRUNCATE);
	if ( strlen( scriptDirectory ) >= sizeof( scriptDirectory ) - 2 ) {
		String^ message = 
			"Invalid Script Directory Path   \nString is too long?\n\n" + scriptRoot + "\n\nExiting program.";
		MessageBox::Show( message, "Fatal Error", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
		return 0;
	}

	// Create pictureFilenamePrefix
	// Pictures are stored in the "pictures" subdirectory to the script directory.
	if ( strlen( scriptDirectory ) >= sizeof( scriptDirectory ) - sizeof( "pictures\\" ) ) {
		String^ message = 
			"Invalid Picture Directory Path   \nString is too long?\n\n" + scriptRoot + "pictures\\ \n\nExiting program.";
		MessageBox::Show( message, "Fatal Error", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
		return 0;
	}
	strcpy( pictureFilenamePrefix, scriptDirectory );
	strcat( pictureFilenamePrefix, "pictures\\" );

	// Create packetBufferPathRoot
	pinchars = PtrToStringChars( packetRoot );
	converted_chars = 0;
	size_in_bytes = ((packetRoot->Length + 1) * 2);
	err = wcstombs_s(&converted_chars, 
			packetBufferPathRoot, sizeof( packetBufferPathRoot ),
			pinchars, _TRUNCATE);

	// Before we do anything else, check if packets are available in the
	//  cache directory. If not, there is nothing to display.
	// Here we show a dialog while we are waiting for some packets to appear.
	GripMMIStartup^ startupForm = gcnew GripMMIStartup( GripMMIVersion );
	System::Windows::Forms::DialogResult result = startupForm->ShowDialog();
	if ( result != System::Windows::Forms::DialogResult::Cancel ) {
		// Create the main window and run it
		Application::Run(gcnew GripMMIDesktop());
	}
	return 0;
}
