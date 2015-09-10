WCM Cost Sample
================================
The sample demonstrates how to set and get cost for WLAN profiles using WCM 
cost APIs.

The APIs demonstrated in this sample are:
1.WcmSetProperty - to set cost or dataplan status for a profile.
2.WcmQueryProperty - to query cost or dataplan status info for a profile.
   
Sample Language Implementations
===============================
 	This sample is available in the following language implementations:
C++
 
Files:
=============================================
Wcmcostsample.cpp 
	This file includes the functions to demonstrate how to use Wcm Set/Query 
	APIs to get cost and dataplan status.

Utils.cpp
	This file contains the utility functions required for the WcmCostSample SDK. 

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the directory containing the sample for a specific language.
     2. Type "msbuild wcmcostsample".


To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open File Explorer and navigate to the directory containing the sample for CPP language.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or File Explorer.
     2. Type wcmcostsample at the command line, or double-click the icon for wcmcostsample to launch it from File Explorer.
