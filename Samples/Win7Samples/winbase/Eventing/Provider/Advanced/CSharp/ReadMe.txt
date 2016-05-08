====================================================================================
ETW MANIFEST-BASED ADVANCED PROVIDER SAMPLE
====================================================================================

Sample Language Implementations
===============================
   This sample is available in the following language implementations:
   C#.


FILES
=================================================
AdvancedProvider.cs
   Main program. Calls generated methods in AdvancedProviderEvents.cs to log ETW events. 

AdvancedProvider.man
   ETW manifest file. Defines ETW provider, provider events and their payloads.

RunE2E.cmd
   Defines scripts to test the application. The script should be run from the project directory. Refer to the last section for detail on this.


Tools used in this sample development
=================================================

- MC.EXE
     Used to generated resource files and event logging methods. Refer to Build Section for more detail. 

- Visual Studio 2008
     Used to build the project from Visual studio. 


BUILD
====================================

Using Visual Studio
  
Use the provided solution file to build the project from Visual Studio 2008. The following configurations have been applied to the project:
   
1. Configure a Pre-Build Event to use MC.EXE and RC.EXE

•	Double click the project solution file to open in Visual Studio
•	Right click on the Project 
•	Select Properties 
•	Under Build Events tab, type the following commands in the Pre-build event command line text box.

MC.EXE "$(ProjectDir)AdvancedProvider.man" -cs AdvancedProvider -h ..\ -z AdvancedProviderEvents
RC.EXE -r AdvancedProviderEvents.rc

NOTE that you may need to run "Windows SDK Configuration Tool" after installing the SDK. 
Otherwise Visual Studio may not be able to find mc.exc and rc.exe.

2. Setup Output directory

•	Under Build tab on the project properties window, set project output path to "Output".  Visual Studio uses this folder as the current working directory when you build the project. This makes sure that MC.exe and RC.exe generated files will go to a subfolder called Output.

3. Set resource file to "Output\AdvancedProviderEvents.res".

•	Under Application tab, set Resource File to "Output\AdvancedProviderEvents.res". AdvancedProviderEvents.res is generated when RC is run as a pre-build event.

The following files will be generated every time the project is built:

•	AdvancedProviderEvents.cs - generated in the project directory (-h switch) and is referenced by AdvancedProvider.cs. It defines methods for ETW Event registration and EventWrites based on the provided manifest.
•	AdvancedProviderEvents.rc - generated in the current directory.
•	AdvancedProviderEventsTEMP.BIN - generated in the current directory.
•	AdvancedProviderEvents_MSG00001.bin - generated in the current directory.
•	AdvancedProviderEvents.res - generated in the current directory.


INSTALL/DEPLOY, UNINSTALL and VIEWING/CONSUMPTION
===============================================
RunE2E.cmd is a script which copies, installs and runs the sample on the same computer where you had built the project. If you want to test the sample on a separate machine, make sure you follow the steps in the RUNE2E.cmd file.

The three important files to test the sample after a successful build are:
•	AdvancedProvider.exe
•	AdvancedProvider.man
•	RunE2E.cmd


More on RUNE2E.cmd 

1.	Copy binary and manifest

md %systemdrive%\Eventing\Advanced\CSharp >NUL 2>&1
copy outPut\AdvancedProvider.exe %systemdrive%\Eventing\Advanced\CSharp
copy AdvancedProvider.man %systemdrive%\Eventing\Advanced\CSharp
pushd %systemdrive%\Eventing\Advanced\CSharp

Because of the absolute path requirement for the message file path and resource file path, this sample chooses %systemdrive%\Eventing\Advanced\CSharp as install/deploy location for the executable and hence the build results should be copied from Output directory to %systemdrive%\Eventing\Advanced\CSharp and then pushes the current directory to %systemdrive%\Eventing\Advanced\CSharp so that the following scripts could be Advanced.

2.	Install the provider using wevtutil.exe (inbox tool)

wevtutil im AdvancedProvider.man

3.	Start the logging session for AdvancedProvider.exe using logman.exe (inbox tool)

logman start -ets AdvancedProvider -p "Microsoft-Windows-SDKSample-AdvancedProvider-CS" 0 0 -o AdvancedProvider.etl 

4.	Execute the provider

        AdvancedProvider.exe

5.	Stop the provider session  

logman stop AdvancedProvider -ets

6.	Generate dumpfile using tracerpt (inbox tool).

tracerpt -y AdvancedProvider.etl 

7.	Uninstall the provider

wevtutil um AdvancedProvider.man

8.	Open dumpfile.xml

notepad dumpfile.xml

Note: You can also use event viewer to verify if the events from this provider have been logged.

