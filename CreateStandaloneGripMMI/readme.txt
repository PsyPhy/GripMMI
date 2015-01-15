========================================================================
    MAKEFILE PROJECT : CreateStandaloneGripMMI Project Overview
========================================================================

Copyright (c) 2014, 2015 PsyPhy Consulting

BuildStandaloneGripMMI.bat
	
	A DOS batch file that copies the latest version of the executables 
	generated here. One can then execute the system outside of the VC++ 
	environment to verify standalone functionality.

CleanStandaloneGripMMI.bat
	Clean the GripExecutables directory.

CreateStandaloneGripMMI.vcxproj
    This is the main project file for VC++ projects generated using an Application Wizard. 
    It contains information about the version of Visual C++ that generated the file, and 
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

CreateStandaloneGripMMI.vcxproj.filters
    This is the filters file for VC++ projects generated using an Application Wizard. 
    It contains information about the association between the files in your project 
    and the filters. This association is used in the IDE to show grouping of files with
    similar extensions under a specific node (for e.g. ".cpp" files are associated with the
    "Source Files" filter).

This project allows you to build/clean/rebuild from within Visual Studio by calling the commands you have input 
in the wizard. The build command can be nmake or any other tool you use.

/////////////////////////////////////////////////////////////////////////////
