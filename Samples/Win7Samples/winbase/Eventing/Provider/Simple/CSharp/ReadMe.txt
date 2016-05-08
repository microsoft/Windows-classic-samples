====================================================================================
ETW MANIFEST-BASED SIMPLE EVENT PROVIDER SAMPLE
====================================================================================

Sample Language Implementations
===============================
   This sample is available in the following language implementations:
   C#.

FILES
=================================================
SimpleProvider.cs
   Main program. Calls generated methods in SimpleProviderEvents.cs to log the event data. 

SimpleProvider.man
   ETW manifest. Defines ETW provider, provider events and their payloads.

RunE2E.cmd
   Command-line script to deploy and test the application. This script should be run from the project directory. Refer to the last section for details on this.


Tools used in this sample development
=================================================

- MC.EXE
     Used to generate resource files and event logging macros. Refer to Build Section for more details. 

- Visual Studio 2008
     Used to build the project. 


BUILD
====================================

Using Visual Studio
  
Use the provided solution file to build the project from Visual Studio 2008. The following configurations have been applied to the project:
   
1. Configure a Pre-Build Event to use MC.EXE and RC.EXE

•	Double click the project solution file to open in Visual Studio
•	Right click on the Project 
•	Select Properties 
•	Under Build Events tab, type the following commands in the Pre-build event command line text box.

MC.EXE $(ProjectDir)SimpleProvider.man -cs SimpleProvider -h ..\ -z SimpleProviderEvents
RC.EXE -r SimpleProviderEvents.rc

NOTE that you may need to run "Windows SDK Configuration Tool" after installing the SDK. 
Otherwise Visual Studio may not be able to find mc.exc and rc.exe.

2. Setup Output directory

•	Under Build tab on the project properties window, set project output path to "Output".  VS uses this folder as the current working directory when you build the project. This makes sure that MC and RC generated files will go to a subfolder called Output.

3. Set resource file to "Output\SimpleProviderEvents.res".

•	Under Application tab, set Resource File to "Output\SimpleProviderEvents.res". SimpleProviderEvents.res is generated when RC is run as a pre-build event.

The following files will be generated every time the project is built:

•	SimpleProviderEvents.cs  - generated in the project directory (-h switch) and is referenced by SimpleProvider.cs. It defines methods for ETW Event registration and EventWrites based on the provided manifest.
•	SimpleProviderEvents.rc  - generated in the current directory.
•	SimpleProviderEventsTEMP.BIN - generated in the current directory.
•	SimpleProviderEvents_MSG00001.bin - generated in the current directory.
•	SimpleProviderEvents.res - generated in the current directory.


INSTALL/DEPLOY, UNINSTALL and VIEWING/CONSUMPTION
===============================================
RunE2E.cmd has defined scripts to copy, install and run the sample on the same box where you ran the project. If you want to run the test on a separate box, make sure you follow the steps in the RUNE2E.cmd file.

The three important files to test the sample after a successful build are:
•	SimpleProvider.exe
•	SimpleProvider.man
•	RunE2E.cmd


More on RUNE2E.cmd. 

1.	Copy binary and manifest

md %systemdrive%\Eventing\Simple\CSharp >NUL 2>&1
copy outPut\SimpleProvider.exe %systemdrive%\Eventing\Simple\CSharp
copy SimpleProvider.man %systemdrive%\Eventing\Simple\CSharp
pushd %systemdrive%\Eventing\Simple\CSharp

Because of the absolute path requirement for message file path and resource file path, this sample chooses “%systemdrive%\Eventing\Simple\CSharp” as install/deploy location for the executable and hence the build results should be copied from “Output” directory to “%systemdrive%\Eventing\Simple\CSharp” and then pushes the current directory to “%systemdrive%\Eventing\Simple\CSharp” so that the following scripts could be simple.


2.	Install the provider using Windows commandline tool wevtutil.exe

wevtutil im SimpleProvider.man

3.	Start the logging session for SimpleProvider.exe using Windows commandline tool logman.exe

logman start -ets SimpleProvider -p "Microsoft-Windows-SDKSample-SimpleProvider-CS" 0 0 -o SimpleProvider.etl 

4.	Execute the provider

        SimpleProvider.exe

5.	Stop the provider session  

logman stop SimpleProvider -ets

6.	Generate a dumpfile using Windows commandline tool tracerpt.exe.

tracerpt -y SimpleProvider.etl 

7.	Uninstall the provider

wevtutil um SimpleProvider.man

8.	Open dumpfile.xml

notepad dumpfile.xml

Note: You can also use event viewer to verify that the events from this provider have been logged.

