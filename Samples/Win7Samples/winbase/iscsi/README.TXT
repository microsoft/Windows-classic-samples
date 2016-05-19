   The iSCSI sample demonstrates the APIs used to configure and manage the MS iSCSI initiator service. iSCSI is a technology that allows an initiator computer to use SCSI devices that reside on another machine across a TCP/IP network. The iSCSI initiator APIs are implemented in iscsidsc.dll and defined in iscsidsc.h. 
   The iSCSICli is a command line utility that allows access to all features of the MS iSCSI initiator service and serves as a guide for how to use the iSCSI Initiator APIs implemented in iscsidsc.dll.

Sample Language Implementations
===============================
    C

Files
=====
    iSCSICLI.sln - This file is the solution file for the sample
    iSCSICli.vcproj - This file is the project file for the solution.  	
    iscsicli.c - This file contains the source code for the sample.

Prerequisites
=============
Make sure MS iSCSI Service is already started prior to trying to run the sample.
To start MS iSCSI service:-
     1. Click on the MS iSCSI Initiator Icon located under Administrative Tools.
     2. Click on Yes button to have the MS iSCSI Service start automatically each time the computer restarts. (Recommended)	


To build the sample using the command prompt:
=============================================
     1. Open the SDK Command Prompt window and navigate to the directory.
     2. Type vcbuild iscsicli.vcproj 


To build the sample using Visual Studio 2008 (preferred method):
================================================================
     1. Open Windows Explorer and navigate to the directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
==================
     1. On an elevated command prompt navigate to the directory that contains the iSCSICli.exe.
     2. Type” iSCSICli /?” at the command line to see details how to execute the iSCSICli sample with different options.
