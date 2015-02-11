#pragma once
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

/// Edit GripMMIVersionControl.c just prior to compiling a new release.


#ifdef __cplusplus
extern "C" {
#endif

extern char *GripMMIVersion;
extern char *GripMMIBuildInfo;

#ifdef __cplusplus
}
#endif