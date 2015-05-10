@echo off
REM This is a DOS batch file used to CLEAN the standalone directory.
REM Destination is the 'GripExecutables' directory relative to this one.
set DEST=..\..\GripExecutables

REM In fact, there are no intermediate files to delete when doing a clean build.
REM Each build will delete all executables in the destination directory  and
REM then copy in the new versions. We want the GripExecutables directory to 
REM have the valid executables left in place when cleaing the project 
REM intermediate files, so there is nothing more to do in the GripExecutables directory.

REM We do, however, want to clean out the directory that holds the cache files
REM during testing so that these working files are not included in a software distribution.
set CACHE=%DEST%\..\GripCache
REM Make sure that there is something to delete.
echo Dummy file. > %CACHE%\dummy.gpk
REM Now delete the cache files.
del %CACHE%\*.gpk



