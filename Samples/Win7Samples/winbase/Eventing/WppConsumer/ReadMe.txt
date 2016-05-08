====================================================================================
WPP EVENT CONSUMER SAMPLE
====================================================================================

Sample Language Implementations
===============================
   This sample is available in the following language implementations:
   C++.

FILES
=================================================
main.cpp
   Main program. Allows user to specify an etl file with WPP events and the corresponding tmf file, required for decoding the events.

common.h
   Header file which includes the necessary libraries, and user defined structure that represents the processing context.
   
Note: Output files are generated in a subfolder called "Output".


Tools used for this sample's development
=================================================

- SDK Build Environment
   Used to build the project from console. Refer to Build section for more detail.

- Visual Studio
   Used to build the project from Visual studio. Refer to Build #2 section for instructions if you are using Visual studio


BUILD
====================================
1. To build the sample using the command prompt:
     • Open the Command Prompt window and navigate to the  directory.
     • Type "msbuild wppdumper.sln". The application will be built in the default \Output directory.

2. To build the sample using Visual Studio:
     • Open Windows Explorer and navigate to the  directory.
     • Double-click the icon for the WppDumper.sln (solution) file to open the file in Visual Studio.
     • In the Build menu, select Build Solution. The application will be built in the \Output directory.
     
3. To build the sample using makefile:
     • Open the Command Prompt window and navigate to the  directory.
     • Type "nmake". The application will be built in the default \Output directory.


VIEWING/CONSUMPTION
===============================================
The important file to test the sample after a successful build is:
     •	WppDumper.exe
     To run the sample:
	 1. Navigate to the \Output directory which contains the new executable using the command prompt.
	 2. Type "wppdumper.exe <etl file> <tmf file>", where <etl file> is the path to the etl file you want to consume
	    the events from, and <tmf file> is the path to the tmf file which contains the description for the events.