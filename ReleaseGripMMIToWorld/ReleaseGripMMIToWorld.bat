echo This is a DOS batch file used to export the current release to the world.
echo on
echo %Cd%
set DEST=..\..\GripExecutables
set TIMESTAMP=%DEST%\ReleaseTimestamp.txt
echo Unfinished > %TIMESTAMP%
copy ..\Release\CLWSemulator.exe %DEST%
copy ..\Release\GripGroundMonitorClient.exe %DEST%
copy ..\Release\GripMMI.exe %DEST%
echo %date% %time% > %TIMESTAMP%
dir %DEST%

