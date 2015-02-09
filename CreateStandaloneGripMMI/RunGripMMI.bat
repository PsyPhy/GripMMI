REM **************************************************************************

REM GripMMI Startup Script.
REM Copyright (c) 2014, 2015 PsyPhy Consulting
REM All rights reserved.

REM Starts up the various modules that make up the GripMMI, 
REM  including a CLSW Server emulator for testing.

REM **************************************************************************

REM Edit the definitions in this section according to the installation.

REM Comment out the next line to disable the CLWS emulator.
set EMULATE=TRUE

REM Directory holding cache files
set CacheDir=..\GripCache\

REM Host Name or Address of the CLWS Server.
REM Can be 'localhost' if running on same machine.
REM Should be 'localhost' if running the CLWSemulator (see below).
set HOST=localhost

REM Directory holding the Grip scripts (.dex)
set ScriptDir=..\GripScripts\

REM Uncomment the next line to use the alternate Software Unit ID.
REM set UNIT=-alt

REM Uncomment the next line if you want to display times in UTC rather than GPS
REM Default value is -16 which corrects for leap seconds as of Jan. 2015.
REM Modify the value if new leap seconds have been added (e.g. after July 2015 change to -17).
set TIMEBASE_CORRECTION=-16

REM Uncomment the next line if you want to display in GPS time.
REM set TIMEBASE_CORRECTION=0

REM **************************************************************************

REM Normally you will not need to edit below this line, 

REM Root of the file names for the cache files
set TIMESTAMP=
set CacheRoot=GripPackets.%date:~10,4%.%date:~4,2%.%date:~7,2%

echo %cd%

REM Start up the emulator if you want to test without the actual GRIP
REM  or EPM hardware. Comment out or delete for real operations.
if not defined EMULATE goto LAUNCH
start .\CLWSemulator.exe

:LAUNCH
REM Launch the TCP/IP Client.
REM This module receives packets from the CLWS server, selects out the 
REM  Grip packets and sends them to intermediate cache files.
REM First parameter is the path and root for the cache files.
REM Second is the host name or IP address in dot format of the CLWS server.
start .\GripGroundMonitorClient.exe %CacheDir%%CacheRoot% %HOST% %UNIT%

REM Launch a 'lite', text-only version of the MMI, just to see if 
REM  the servers are running and the cache files are filling up.
REM Usually this is disables by commenting the next line.
REM start GripMMIlite.exe %CacheDir%\%CacheRoot%

REM Start the VC++ 6 version of the Grip MMI.
REM First parameter is the path to the packet caches that serve as inputs.
REM Second paramter is the path to the scripts that are installed on board.
REM start GripGroundMonitor.exe %CacheDir%\%CacheRoot% %ScriptDir% 

REM Start the actual graphical GripMMI.
REM First parameter is the path to the packet caches that serve as inputs.
REM Second paramter is the path to the scripts that are installed on board.
start .\GripMMI.exe %CacheDir%\%CacheRoot% %ScriptDir% %TIMEBASE_CORRECTION%
