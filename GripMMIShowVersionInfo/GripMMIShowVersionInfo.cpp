// GripMMIShowVersionInfo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../GripMMIVersionControl/GripMMIVersionControl.h"


int _tmain(int argc, _TCHAR* argv[])
{
	printf( "%s %s", GripMMIVersion, GripMMIBuildInfo );
	return 0;
}

