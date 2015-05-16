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
REM Note that you should include a final backslash.
set CacheDir=..\GripCache\

REM
REM Directory holding the Grip scripts (.dex)
REM

REM The client program will look for the GRIP scripts here.
REM Again, you should include a terminating backslash.
set ScriptDir=..\GripScripts\

REM 
REM Location of restart batch file.
REM

REM When the current script is run, it automatically generates a batch file
REM that makes it easy to relaunch the graphical interface without restarting
REM the client process. This batch file must be written in a write-enabled directory.
REM Logically, I would put it in the current directory, because that is where 
REM RunGripMMI.bat (this file) has been found and executed. The corresponging assign would be:
REM
REM set RestartDir=.\
REM
REM But this directory is probably write protected at CADMOS. 
REM So I write it to the cache directory instead:
set RestartDir=%CacheDir%

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

REM Uncomment the next line if you want to display times aligned with UTC rather than GPS
set TIMEBASE_CORRECTION=-16
REM Uncomment the next line if you want to display times aligned with GPS time.
REM set TIMEBASE_CORRECTION=0

REM **************************************************************************

REM Create a common timestamp in the form YYYY.MM.DD to be used in various file names.

REM Hopefully you will not need to edit this section, but it depends on the Windows configuration.
REM Here we extract the date from the environment variable %date%. But I have seen two
REM different formats:  "DD/MM/YYYY" and "Day MM/DD/YYYY". Here I try to identify which one it is
REM and extract accordingly. But you may need to further modify depending on the 
REM format of the %date% variable on your system. To find out, type "echo %date%" in a command window.

REM If the 3rd character is a '/' then we have the "DD/MM/YYYY" format.
if "/"=="%date:~2,1%" goto SHORTDATE
REM If we are here, we presume that we have the "Day MM/DD/YYYY" format.
set TIMESTAMP=%date:~10,4%.%date:~4,2%.%date:~7,2%
goto NEXT
:SHORTDATE
set TIMESTAMP=%date:~6,4%.%date:~3,2%.%date:~0,2%
:NEXT

REM **************************************************************************

REM Normally you will not need to edit below this line.

REM Root of the file names for the cache files
set CacheRoot=GripPackets.%TIMESTAMP%

REM Show where we are for debugging purposes.
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
REM The process is launched with /REALTIME priority to help ensure that
REM  packets from the EPM server are not missed.
start /REALTIME .\GripGroundMonitorClient.exe %CacheDir%%CacheRoot% %HOST% %UNIT%

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

REM Now create a batch file that will make it easier to restart the graphical
REM GripMMI.exe interface without restarting GripGroundMonitorClient.exe

set restart_file=%RestartDir%\RestartGripMMI.%TIMESTAMP%.bat
echo REM > %restart_file%
echo REM %restart_file% >> %restart_file%
echo REM >> %restart_file%
echo REM A batch file to restart the GripMMI graphical interface. >> %restart_file%
echo REM This will restart GripMMI.exe with the most recently defined parameters. >> %restart_file%
echo REM It assumes that GripGoundMonitorClient.exe is still running. >> %restart_file%
echo REM If GripGoundMonitorClient.exe is not running, use RunGripMMI.bat instead. >> %restart_file%
echo REM >> %restart_file%
echo REM This batch file can also be used to review previously recorded data >> %restart_file%
echo REM using the GripMMI graphical interface. >> %restart_file%
echo REM >> %restart_file%
echo CD %CD% >> %restart_file%
echo start .\GripMMI.exe %CacheDir%\%CacheRoot% %ScriptDir% %TIMEBASE_CORRECTION% >> %restart_file%
