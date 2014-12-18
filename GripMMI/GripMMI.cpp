// GripMMI.cpp : main project file.

#include "stdafx.h"
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
	if ( args->Length > 0 ) packetRoot = args[0];
	if ( args->Length > 1 ) scriptRoot = args[1];

	// Create the main window and run it
	Application::Run(gcnew GripMMIDesktop( packetRoot, scriptRoot ));
	return 0;
}
