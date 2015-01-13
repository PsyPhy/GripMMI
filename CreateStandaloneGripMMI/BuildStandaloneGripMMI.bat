echo This is a DOS batch file used to export the current executables to the standalone directory.

REM Follow progress during a build.
echo CD =      %cd%

REM Destination is the 'GripExecutables' directory relative to this one.
set DEST=..\..\GripExecutables
echo DEST =   %DEST%

REM Source depends on the first command line argument.
REM Should be 'Debug' or 'Release'
set CONFIG=%1
echo CONFIG = %CONFIG%

REM Create a timestamp so that we know when this standalone was created.
REM Make sure that the old timestamp doesn't get left in place if the
REM build fails.
set TIMESTAMP=%DEST%\Timestamp.txt
echo Unfinished > %TIMESTAMP%

REM Copy the critical executables to the destination.
REM We include only the executables that are needed for installation
REM on a client machine, not all of the test executables.
echo copy ..\%CONFIG%\CLWSemulator.exe %DEST%
copy ..\%CONFIG%\CLWSemulator.exe %DEST%
echo copy ..\%CONFIG%\GripGroundMonitorClient.exe %DEST%
copy ..\%CONFIG%\GripGroundMonitorClient.exe %DEST%
echo copy ..\%CONFIG%\GripMMI.exe %DEST%
copy ..\%CONFIG%\GripMMI.exe %DEST%

REM Copy the batch file used to launch the executables.
REM We keep the source copy of the file here so as to track changes.
copy RunGripMMI.bat %DEST%

REM Show the latest build info.
set BUILDINFO=..\LatestBuildInfo.txt
..\%CONFIG%\GripMMIShowVersionInfo > %BUILDINFO%
echo copy %BUILDINFO% %DEST%
copy %BUILDINFO% %DEST%

REM Presumably, we finished correctly. Update the timestamp.
echo %date% %time% %CONFIG% > %TIMESTAMP%