XA Superior Resource Manager Sample
===================================

This sample demonstrates how to build an XA Superior resource manager.
The sample connects to the Distributed Transaction Coordinator (DTC) as its subordinate resource manager and 
completes an XA transaction.



Requirements
============
1. DTC service must be running, and be configured to allow XA transactions.

Note: 	Read Component Services MMC Snapin help documentation for instructions on how to configure DTC
	You can see the Component services MMC Snapin documentation by following these steps:
	1. Run 'dcomcnfg' at the command line
	2. Click 'Help' from the menu, Click 'Help topics'



Sample Source Files
===================
Readme.txt		This file
XASuperior.cpp		Main program
XASuperior.sln		Visual Studio solution file
XASuperior.vcproj	Visual C projet file



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
Run XASuperior.exe from the command line.



Supported Platforms
===================
Windows XP SP2
Windows 2003 Server
Windows Vista
Windows Server 2008