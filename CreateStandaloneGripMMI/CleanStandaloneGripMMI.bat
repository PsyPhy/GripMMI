@echo off
REM This is a DOS batch file used to CLEAN the standalone directory.
REM Destination is the 'GripExecutables' directory relative to this one.
set DEST=..\..\GripExecutables
set CACHE=%DEST%\..\GripCache

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
echo Dummy file. > %DEST%\RunGripMMI.bat
del %DEST%\RunGripMMI.bat

REM Clean the packet cache directory as well.
set CACHE=%DEST%\..\GripCache
echo Dummy file. > %CACHE%\dummy.gpk
del %CACHE%\*.gpk



