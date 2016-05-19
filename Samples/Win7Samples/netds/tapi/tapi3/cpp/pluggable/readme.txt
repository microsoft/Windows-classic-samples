--------------------------------------------------------------------------

  Copyright (C) 1998-2003 Microsoft Corporation. All rights reserved.

--------------------------------------------------------------------------

TAPI 3.1 Pluggable Terminals Sample Application


Overview:
~~~~~~~~~

PlgTerm is a sample TAPI 3.1 application that shows how to create a pluggable
terminal. It uses MSPBase classes and DirectShow\BaseClasses.

PLUGGABLE.CPP shows how to register the terminal for TerminalManger and
and how to register the COM component.

PLGTERM.CPP shows the implementation of the virtual or pure vitual methods
which a terminal must or may implement.
 
PLGTERMFILTER.CPP shows the implementation of the terminal filter.
PLGTERMPIN.CPP shows the implementation of the pin on the terminal filter.

The terminal is a render terminal and it would print the size of the received 
sample after the terminal was selected on an active stream.

It need TAPI3.1 and will run only under Windows XP.

How to build the sample:
~~~~~~~~~~~~~~~~~~~~~~~~

Set the SDK build environment

Before building the PLGTERM sample application you need to 
	-install the DirectX9 SDK
	-build the $(MSSDK)\samples\Multimedia\DirectShow\BaseClasses sample
	to produce UNICODE strmbase.lib or UNICODE strmbasd.lib file before building PlgTerm sample 
		-Set the build configuration of the BaseClasses sample to "Debug Unicode" or "Release Unicode" 
		using Visual Studio's build configuration manager to build UNICODE libraries
	-run the setenv.bat file in the Platform SDK root dir
	
	-build $(MSSDK)\Samples\NetDS\Tapi\Tapi3\Cpp\Msp\MSPBase sample to produce MSPBaseSample.lib
	Type nmake in the MSPBase directory

If all the necessary are built correctly you may build the PLGTERM sample application.
Type "nmake" in the PLGTERM directory.  This will build plgterm.dll.
This sample uses DirectShow\BaseClasses so you need to have Multimedia samples
installed in order to build this sample.


How to use the sample:
~~~~~~~~~~~~~~~~~~~~~~

After the sample is built, you'll need to register the terminal 
"regsvr32 plgterm.dll" in order to use it.

After create a TAPI3 call you can select the terminal on a render audio stream.
After call is connected and the streams is active it will log the samples size

What functionality does this sample show:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The incoming sample application demonstrates how to use pluggable terminals 
mechanism and how to build a terminal using MSPBase classes

What this sample does not show:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This sample does not show how the received samples are used.

Hints:
~~~~~~

If you do not want this terminal to show up in the list of TAPI3.1 terminals 
you need to deregister it - "regsvr32 /u plgterm.dll"

Additional Notes on how to build the sample: 

 How to build the samples with VS.Net or VC6 or VC5:
-	install DirectX9 SDK
-	build the $(MSSDK)\samples\Multimedia\DirectShow\BaseClasses 
	DirectShow sample.  Build UNICODE (debug or release or both) configurations
	using the Visual studio Envoriment
-	go to the path where you installed the platform SDK 
	(e.g. C:\Program Files\Microsoft SDK) 
	and type SetEnv.Bat.
-	check the following environment variables: 
	PATH, LIB, INCLUDE. You can see their current 
	values by typing "SET" at the command prompt. 
	You should see that they contain first the SDK 
	paths and then the VC6 paths.
-       Set the enviroment variable DXSDK_DIR to point to the 
	root directory of the DirectX9 SDK
	(e.g. set DXSDK_DIR=C:\DXSDK )
       NOTE: With latest Direct X SDK(DirectX 9.0 SDK -August 2005) , DXSDK_DIR is already set
              set it manually only if the env var is not already set

	You can add this to the setenv.bat so DXSDK_DIR will be set 
	whenever you run setenv.bat.  A good place to add this 
	line is right after the set MSSDK= line
-	go to the path where the PlgTerm sample is installed
	(Samples\NetDS\Tapi\Tapi3\cpp\Pluggable) and type NMAKE. 
