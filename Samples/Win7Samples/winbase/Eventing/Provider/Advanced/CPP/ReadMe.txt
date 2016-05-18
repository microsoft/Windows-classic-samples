====================================================================================
ETW MANIFEST-BASED ADVANCED EVENT PROVIDER SAMPLE
====================================================================================

Sample Language Implementations
===============================
   This sample is available in the following language implementations:
   C++.


FILES
=================================================
AdvancedProvider.cpp
   Main program. Calls generated macros in AdvancedProviderEvents.h to log 75 different event data. 

AdvancedProvider.man
   ETW manifest. Defines ETW provider, provider events and their payloads.

AdvancedProvider.rc
   Defines application version and manifest information.

makefile
   Defines build and compilation options. It is used to build the project from  Microsoft Platform SDK build environment. This file contains scripts to build the executable. In particular, the definition has the following sections:
-	A script to run MC.EXE to generate header, message and resource files out of the provided manifest file.
-	A script to generate a RES file using RC.EXE.
-	A script to compile and link all output files.
-	A script to clean up the project directory.

Note: Output files are generated under Debug folder.

RunE2E.cmd
   Defines scripts to test the application. The script should be run from the project directory. Refer to the last section for details on this.


Tools used in this sample development
=================================================

- MC.EXE
     Used to generate resource files and event logging macros. Refer to Build Section for more details. 

- SDK Build Environment
     Used to build the project from console. Refer to Build section for more details.

- Visual Studio 2008
     May be used to build the project. Refer to Build #2 section for instructions if you are using Visual Studio.


BUILD
====================================
1. Using SDK Build Environment

1.1 Use nmake command to build the project under the  Microsoft Platform SDK build environment. Output files are created in a subfolder called "Debug". 
This procedure uses the makefile definition to build the project. 

1.2 Use MSBuild command to build the project under the Microsoft Platform SDK build environment.  
	MSBuild AdvancedProvider.sln

2. Using Visual Studio 
Open the provided solution file in Visual Studio 2008 and choose Build -> Build solution. 
Note: The project is configured to run MC.EXE automatically as a custom build step. The custom build step can be viewed/modified as follows:     
•	From Solution Explorer, Right click on the AdvancedProvider.man
•	Select Properties 
•	Select Custom Build Step under Configuration Properties 
•	Verify that the following scripts exist.
    Command Line:  mc.exe -um $(InputFileName) -z $(InputName)Events -h $(OutDir) -rc $(OutDir)
    Outputs:  $(OutDir)\$(InputName)Events.h

The custom build step generates the following files when the project is built:
•	AdvancedProviderEvents.h  - defines macros for ETW Event registration and Event writes based on the provided manifest.
        Note: This file is included in AdvancedProvider.cpp file.
•	AdvancedProvider.res  - resource file.

INSTALL/DEPLOY, UNINSTALL and VIEWING/CONSUMPTION
==============================================================
RunE2E.cmd can be used to run the sample on the same machine where the project is built. 

The three important files to test the sample after a successful build are:
•	AdvancedProvider.exe
•	AdvancedProvider.man
•	RunE2E.cmd

More on RUNE2E.cmd. 

1.	Copy binary and manifest
md %systemdrive%\Eventing\Advanced\CPP >NUL 2>&1
copy [OutPut]\AdvancedProvider.exe %systemdrive%\Eventing\Advanced\CPP
copy AdvancedProvider.man %systemdrive%\Eventing\Advanced\CPP
pushd %systemdrive%\Eventing\Advanced\CPP

Note: 
- [OutPut] is either "Debug" or "Release" depending on the solution configuration of the project in Visual Studio. The default is "Debug"
- The binary should be copied to "%systemdrive%\Eventing\Advanced\CPP" because the same path is referenced as the resourceFilepath and messageFilepath in the manifest. 

2.	Install the provider using Windows commandline tool wevtutil.exe
wevtutil im AdvancedProvider.man

3.	Start the logging session for AdvancedProvider.exe using Windows commandline tool logman.exe
logman start -ets AdvancedProvider -p "Microsoft-Windows-SDKSample-AdvancedProvider" 0 0 -o AdvancedProvider.etl 

4.	Execute the provider
AdvancedProvider.exe

5.	Stop the provider session  
logman stop AdvancedProvider -ets

6.	Generate a dumpfile using Windows commandline tool tracerpt.exe
tracerpt -y AdvancedProvider.etl 

7.	Uninstall the provider
wevtutil um ..\AdvancedProvider.man

8.	Open dumpfile.xml
notepad dumpfile.xml

Note: You can also use event viewer to verify that the events for this provider have been logged.

