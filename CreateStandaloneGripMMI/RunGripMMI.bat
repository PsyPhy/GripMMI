REM **************************************************************************

REM GripMMI Startup Script.
REM Copyright (c) 2014, 2015 PsyPhy Consulting
REM All rights reserved.

REM Starts up the various modules that make up the GripMMI, 
REM  including a CLSW Server emulator for testing.

REM **************************************************************************

REM Edit the definitions in this section according to the installation.

REM
REM CLWS Emulator for testing
REM 

REM The batch file starts up, by default, a CLWS emulator for local testing.
REM For final installation, comment out the next line to disable the CLWS emulator.
set EMULATE=TRUE

REM The next lines determine the mode of the CLWS emulator.
REM It is currently configured to send out pre-recorded packets.
REM set EMULATE_MODE=-constructed
set EMULATE_MODE=-recorded

REM You can substitute a different file as the source of the recorded packets
REM  by changing the following definition. Note that this definition has
REM  no effect if you are in "-constructed" mode.
set PACKET_SOURCE=".\\GripPacketsForSimulator.gpk"

REM Host Name or Address of the CLWS Server.
REM Can be 'localhost' if running on same machine.
REM Should be 'localhost' if running the CLWSemulator (see above).
set HOST=localhost

REM
REM Directory holding cache files
REM

REM The client program will store packets received from the CLWS in this directory.
set CacheDir=..\GripCache\

REM
REM Directory holding the Grip scripts (.dex)
REM
set ScriptDir=..\GripScripts\

REM
REM Alternate Software Unit ID.
REM
REM The CLWS can accept only one connection from a given Software Unit.
REM If you want to use two client connected to the same CLWS server,
REM  uncomment the next line to use the alternate Software Unit ID.
REM set UNIT=-alt

REM 
REM UTC versus GPS time of day.
REM

REM EPM used GPS time-of-day, but CADMOS displays UTC time. There is a small shift between them,
REM  because UTC takes into account leap seconds, while GPS does not.
REM Default value is -16 which corrects for leap seconds as of Jan. 2015.
REM Modify the value if new leap seconds have been added (e.g. after July 2015 change to -17).

REM Uncomment the next line if you want to display times in UTC rather than GPS
set TIMEBASE_CORRECTION=-16
REM Uncomment the next line if you want to display in GPS time.
REM set TIMEBASE_CORRECTION=0

REM **************************************************************************

REM Normally you will not need to edit below this line, 

REM Root of the file names for the cache files
set TIMESTAMP=
set CacheRoot=GripPackets.%date:~10,4%.%date:~4,2%.%date:~7,2%

echo %cd%

REM Starts up the emulator according to configuration defined above.
if not defined EMULATE goto LAUNCH
REM CLWSemulator takes a mode flag, -constructed or -recorded, and
REM  an optional path to the pre-recorded EPM packets.
start .\CLWSemulator.exe %EMULATE_MODE% %PACKET_SOURCE%

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
