
UPNPSampleDimmerDevice


Overview:
=========

UPNPSampleDimmerDevice implements the dimmer device and dimmer service functionality of the 
RegDevice sample. This is implemented as COM server object.


Files:
======

DeviceDll.cpp		- Implementation of DLL Exports from DeviceDll.def
DeviceDll.def		- DLL exports for UPNPSampleDimmerDevice.dll
DeviceDll.h		- COM server module declaration
DimmerDevice.idl	- Dimmer Service component and library descriptions
DimmerDeviceDCO.cpp	- Implementation code for Dimmer Device
DimmerDeviceDCO.h	- Class definitions for implementation of Dimmer Device
DimmerDeviceDCO.rgs	- Registration script file to register the COM server object
DimmerService.cpp	- Implementation code for Dimmer Service
DimmerService.h		- Class definitions for implementation of Dimmer Service
resource.h		- Resource include file
UPNPSampleDimmerDevice.rc - Resource information for this project 


Building the Sample:
====================

To build the sample using the command prompt:

     1. Open the Command Prompt window and navigate to the DCO_DimmerService directory.
     2. Type 'msbuild UPNPSampleDimmerDevice.sln'

To build the sample using Visual Studio 2008 (preferred method):

     1. Open Windows Explorer and navigate to the DCO_DimmerService directory.
     2. Double-click the icon for the UPNPSampleDimmerDevice.sln (solution) file to open 
        the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application (UPNPSampleDimmerDevice.dll)
        will be built in the default \Debug or \Release directory.


Running the Sample:
===================

This is a part of the RegDevice sample. Please refer to the readme.txt under 
RegDevice for instructions on how to run the RegDevice sample.
