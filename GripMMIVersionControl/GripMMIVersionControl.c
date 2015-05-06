///
/// Module:	GripMMIVersionControl (GripMMI)
/// 
///	Author:					J. McIntyre, PsyPhy Consulting
/// Initial release:		18 December 2014
/// Modification History:	see https://github.com/PsyPhy/GripMMI
///
/// Copyright (c) 2014, 2015 PsyPhy Consulting
///

/// This module creates a library at the bottom of the GripMMI hierarchy.
/// When compiled, it defines version and build info for the GripMMI project.
/// Other modules link to it to get the shared version and build info.

/// Edit the lines below prior to compiling a new release.

#ifdef _DEBUG 
char *GripMMIVersion = "GripMMI V1.2.0 (debug)";
char *GripMMIBuildInfo = "Build "__DATE__" "__TIME__;
#else
char *GripMMIVersion = "GripMMI V1.2.0 (beta)";
char *GripMMIBuildInfo = "Build "__DATE__" "__TIME__;
#endif
