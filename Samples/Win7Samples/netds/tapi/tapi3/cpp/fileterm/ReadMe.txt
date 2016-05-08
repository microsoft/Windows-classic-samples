

  Copyright (C) 1998-2003 Microsoft Corporation. All rights reserved.

--------------------------------------------------------------------------

TAPI 3.1 File Terminals Sample Application


Overview:
~~~~~~~~~

FileTerm is a sample TAPI 3.1 application that waits for and answers
incoming calls. It uses the File Terminals to play out a recorded 
message and then records the caller's message to a file.

In order to receive incoming call notifications, as well as any call
state event notifications, the outgoing ITCallNotification interface
must be implemented by the TAPI 3.x application, and registered with
TAPI 3.1 through the IConnectionPoint mechanism.  For more information
on IConnectionPoint, and IConnectiontPointContainer, please refer
to the COM documentation.

CALLNOT.CPP and CALLNOT.H show the implementation of the ITCallNotification
interface.  ITCallNotification is defined by TAPI 3.x, and the interface
definition is in tapi3.h.
FILETERM.CPP shows how to register the interface, and how to answer
an incoming call.

It need TAPI3.1 and will run only under WinNT5.1.

How to build the sample:
~~~~~~~~~~~~~~~~~~~~~~~~

To run the FILETERM sample application, set the SDK build environment, then
type "nmake" in the fileterm directory.  This will build fileterm.exe

How to use the sample:
~~~~~~~~~~~~~~~~~~~~~~

After the sample is built, copy welcome.wav file in same directory with the 
executable file and run FILETERM.EXE . 

A small dialog box will appear, and the status will be "Waiting for a call."
If your computer does not have any TAPI devices, the application will display
an error as is starts up.  The application will wait for calls on all TAPI
addresses that support audio calls.

When a call arrives on one of the TAPI addresses, the application will answer
the call. It will play out a recorded message from a file (welcome.wav also 
an avi fiel can be used - 16 bits, mono, PCM). When playback finishes it will
record the caller's message to a file (messageX.avi - where X is the 
message number).
When the application restarts the number of the first message is 0 - the old
messages are overwritten. The maximum length of a message is 60 seconds.
Only one call can be active at a time - all other calls will be rejected.
A message box will popup if a fatal error was encountered.

What functionality does this sample show:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The incoming sample application demonstrates how to use the 
File Terminals (and related interfaces) in the context of a TAPI call. 
It also demonstrate the use of ITBasicCallControl2 (exposed by the Call object) 
and ITMultiTrackTerminal (exposed by File terminals) interfaces.

What this sample does not show:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This sample does not let the user choose the TAPI address for the
application to use.  It does not allow the user to decide what media
types to listen for, and it does not allow the user to decide what
terminals to use. It doesn't use video in the call.

Hints:
~~~~~~

This sample should be able to run as long as you have TAPI devices
installed.  Many computers have a voice modem.  If the modem is installed 
correctly, it will show up as a TAPI device. Also, there are TAPI
devices corresponding to various IP telephony services that are present on 
most systems.


Additional Notes on how to build the sample: 

 How to build the samples with VS.Net or VC6 or VC5:
-	install DirectX9 SDK
-	go to the path where you installed the platform SDK 
	(e.g. C:\Program Files\Microsoft SDK) 
	and type SetEnv.Bat.
-	check the following environment variables: 
	PATH, LIB, INCLUDE. You can see their current 
	values by typing "SET" at the command prompt. 
	You should see that they contain first the SDK 
	paths and then the VC6 paths.
-	Set the enviroment variable DXSDK_DIR to point to the 
	root directory of the DirectX9 SDK
	(e.g. set DXSDK_DIR=C:\DXSDK )
        NOTE: With latest Direct X SDK(DirectX 9.0 SDK -August 2005) , DXSDK_DIR is already set
              set it manually only if the env var is not already set

	You can add this to the setenv.bat so DXSDK_DIR will be set 
	whenever you run setenv.bat.  A good place to add this 
	line is right after the set MSSDK= line
-	go to the path where the FileTerm sample is installed
	(Samples\NetDS\Tapi\Tapi3\cpp\FileTerm) and type NMAKE. 
