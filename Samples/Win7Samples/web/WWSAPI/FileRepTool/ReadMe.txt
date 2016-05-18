FileRepToolExample

Description
===========

This is the command line tool that drives the (FileRepServiceExample) FileRep service. A more detailed description of this sample can be found (FileRepServiceExample) there.

The command line parameters are as follows:

WsFileRep.exe  <Client Service Url> <Server Service Url> <Source File> <Destination File> [/encoding:<binary/text/MTOM>] [/sync]
Client Service Url:Mandatory. Url of the client service.
Server Service Url:Mandatory. Url of the server service.
Source File:Mandatory. Fully qualified local name of the source file. The file is located on the machine where the server service runs.
Destination file:Mandatory. Fully qualified local name of the destination file. The file will be located on the machine where the client service runs.
Encoding:Optional. Specifies the message encoding for the messages sent between the client and server services. The encoding used for the communication between the command line tool and the client service can not be changed for simplicity reasons. Valid parameters are binary, text and MTOM. If the parameter is not specified the default encoding for transport is used.
Sync:Optional. When set, the request completes synchronously. Otherwise the request will complete asynchronously.


Security Note 
=============


This sample is provided for educational purpose only to demonstrate how to use 
Windows Web Services API. It is not intended to be used without modifications 
in a production environment and it has not been tested in a production 
environment. Microsoft assumes no liability for incidental or consequential 
damages should the sample code be used for purposes other than as intended.

Prerequisites
=============

In order to run this sample on Windows XP, Windows Vista, Windows Server 2003
and Windows Server 2008, you may need to install a Windows Update that contains
the runtime DLL for Windows Web Services API. Please consult with the 
documentation on MSDN for more information.

Building the Sample
===================

To build the FileRepToolExample sample
  1. Open the solution FileRepToolExample.sln in Visual Studio. 
  2. Change the active solution platform to the desired platform in the 
     Configuration Manager found on the Build menu.
  3. On the Build menu, click Build. 

Running the Sample
==================

To run the FileRepToolExample sample
  1. Run FileRepToolExample by clicking Start Without Debugging on the Debug menu.

� Microsoft Corporation. All rights reserved.


