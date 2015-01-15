///
/// Module:	GripMMIShowVersionInfo (GripMMI)
/// 
///	Author:					J. McIntyre, PsyPhy Consulting
/// Initial release:		18 December 2014
/// Modification History:	see https://github.com/frenchjam/GripGroundMonitorClient
///
/// Copyright (c) 2014, 2015 PsyPhy Consulting
///

/// This module creates a console application that simply outputs the current
///  release and build info to stdout. It is used by the CreateStandaloneGripMMI 
///  makefile project to write release information into a human readable text 
///  file to be included with the executables and other files when GripMMI is released.

#include "stdafx.h"
#include "../GripMMIVersionControl/GripMMIVersionControl.h"


int _tmain(int argc, _TCHAR* argv[])
{
	printf( "%s %s", GripMMIVersion, GripMMIBuildInfo );
	return 0;
}

