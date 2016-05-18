Simple Resource Manager Sample
==============================

This sample demonstrates how to build a simple resource manager using the Distributed Transaction 
Coordinator (DTC) interfaces. The sample also shows how to marshal a propagation token to pass
the transaction information from one application to another.

The sample implements a resource manager class and a client. The client creates a transaction and 
asks the resource manager to do work as part of the transaction. The client passes a transaction propagation
token to the resource manager to pass the transaction over.



Requirements
============
1. DTC service must be running.

Note: 	Read Component Services MMC Snapin help documentation for instructions on how to configure DTC
	You can see the Component services MMC Snapin documentation by following these steps:
	1. Run 'dcomcnfg' at the command line
	2. Click 'Help' from the menu, Click 'Help topics'



Sample Source Files
===================
Readme.txt		      This file
SimpleResourceManager.cpp     Main program
SimpleResourceManager.sln     Visual Studio Solution file
SimpleResourceManager.vcproj  Visual C project file



To build the sample
===================

1. Copy the sample files to a seperate folder
2. Run the following command at the command line in the same folder as the sample files
	vcbuild /platform:<str>
   where <str> is x64 or Win32 indicating the platform for which you want to build



To clean the sample
===================

1. Run the following command at the command line in the same folder as the sample files
	vcbuild /clean



To run the sample
=================
Run SimpleResourceManager.exe from the command line.


Supported Platforms
===================
Windows XP SP2
Windows 2003 Server
Windows Vista
Windows Server 2008
