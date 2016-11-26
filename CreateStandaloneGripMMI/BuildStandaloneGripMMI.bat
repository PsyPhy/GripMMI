echo This is a DOS batch file used to export the current executables to the standalone directory.

REM Follow progress during a build.
echo CD =      %cd%

REM Destination is the 'GripExecutables' directory relative to this one.
REM This is where the .exe and .dll files go.
set DEST=..\..\GripExecutables
echo DEST =   %DEST%
dir %DEST%

REM Source depends on the first command line argument.
REM Should be 'Debug' or 'Release'
set CONFIG=%1
echo CONFIG = %CONFIG%

REM Clean any previous executables from the destination directory.
REM Create a dummy executable so that the subsequent delete command
REM  won't complain if it doesn't find any .exe files.
echo Dummy file. > %DEST%\dummy.exe
del /Q %DEST%\*.exe

REM Remove the previous timestamps as well.
echo Dummy file. > %DEST%\Timestamp.txt
del %DEST%\Timestamp.txt
echo Dummy file. > %DEST%\LatestBuildInfo.txt
del %DEST%\LatestBuildInfo.txt

REM Remove the batch file used to launch the executables.
REM It will be recopied from the source file further on.
echo Dummy file. > %DEST%\RunGripMMI.bat
del %DEST%\RunGripMMI.bat

REM Remove also any batch files that were created to be used to restart GripMMI.exe.
REM It is possible that there are none. Depending on how RunGripMMI.bat is configured,
REM the automatically-generated restart batch files might be generated here in the
REM executables directory, or in the Cache directory. Here we remove them from the 
REM executables directory. Doing a 'clean' will remove them from the cache directory.
REM Make sure that there is something to delete.
echo Dummy file. > %DEST%\RestartGripMMI.bat
REM Now delete all .bat files.
del %DEST%\RestartGripMMI*.bat

REM Create a timestamp so that we know when this standalone was created.
REM Here we create a phony timestamp that will let us know if the make failed before completing.
set TIMESTAMP=%DEST%\Timestamp.txt
echo Unfinished > %TIMESTAMP%

REM Copy the critical executables to the destination.
REM We include only the executables that are needed for installation
REM on a client machine, not all of the test executables.
echo copy ..\%CONFIG%\CLWSemulator.exe %DEST%
copy ..\%CONFIG%\CLWSemulator.exe %DEST%
echo copy ..\%CONFIG%\GripGroundMonitorClient.exe %DEST%
copy ..\%CONFIG%\DexGroundMonitorClient.exe %DEST%
echo copy ..\%CONFIG%\GripMMI.exe %DEST%
copy ..\%CONFIG%\GripMMI.exe %DEST%

REM Copy the batch files used to launch the executables.
REM We keep the source copy of the file here so as to track changes.
copy RunGripMMI.bat %DEST%
copy RunGraspMMI.bat %DEST%
copy *.dll %DEST%
copy *.gpk %DEST%

REM Show the latest build info.
set BUILDINFO=..\LatestBuildInfo.txt
..\%CONFIG%\GripMMIShowVersionInfo > %BUILDINFO%
echo copy %BUILDINFO% %DEST%
copy %BUILDINFO% %DEST%

REM Presumably, we finished correctly. Update the timestamp.
echo %date% %time% %CONFIG% > %TIMESTAMP%