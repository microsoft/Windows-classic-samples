FileRepServiceExample

Description
===========

FileRep retrieves files from a server and copies them to a client. To do so it employs three components - the server service running on the machine with the source file, the client service running on the machine where the destination file will be stored and a command line tool to control the copying. The client and server services are constantly running web services while the command line tool is started by the user and exits after one request.

This sample illustrates the use of the channel and serialization layer.

This is the service. The command line tool can be found (FileRepToolExample) here.The service has a client and a server mode, where the server sends files and the client receives files.

The command line parameters for the client mode are as follows:

WsFileRepService.exe client  <Service Url> [/reporting:<error/info/verbose>] [/encoding:<text/binary/MTOM>] [/connections:<number of connections>]
Client:Required. Denotes that the service runs as client.
Service Url:Reqired. Denotes the URL the service listens on.
Encoding:Optional. Specifies the encoding used when communicating with the command line tool. Note that the current tool does not support specifying an encoding for this transfer, so changing this setting will likely produce an error. The setting is there so that the tool can be changed and extended independently of the server.
Reporting:Optional. Enables error, information or verbose  level reporting. The default is error. Messages are printed to the console.
Connections:Optional. Specifies the maximum number of concurrent requests that will be processed. If omitted the default is 100.

The command line parameters for the server mode are as follows:

WsFileRepService.exe server <Service Url> [/reporting:<error/info/verbose>] [/encoding:<text/binary/MTOM>] [/connections:<number of connections>] [/chunk:<size of a the payload per message in bytes>]
Server:Required. Denotes that the service runs as file server.
Chunk:Optional. The transferred files are broken into chunks of the specified size. Each message contains one chunk. The default is 32768 bytes.
Implementation details
The main message processing loop is in CRequest. That class contains the application-independent state and methods needed for an asynchronous WWAAPI messaging processing loop. The application-specific code is in CFileRepClient (client service)  and CFileRepServer (server service). Both those classes inherit from CFileRep, which contains generic service-related code.

The sample both uses the serializer and performs custom serialization. Custom serialization is used when dealing with large chunks of data to minimize memory consumption by manually optimizing memory allocation for the specific purpose. As this leads to complex, low-level code doing manual serialization should only be done when absolutely neccessary.
Message exchange pattern
The client service gets a request message from the command line tool.

If the request is asynchronous send back a confirmation immediately.

The client service sends a request for file information to the server service. A discovery request is denoted by a chunk position of -1.

The server service returns the file information.

The client service requests the individual chunks sequentially one by one from the server. Chunks are identified by their position within the file.

Repeat until the file transfer is completed or a failure occured.

If the request is synchronous send success or failure message to the command line tool.

For the individual data structures associated with each message, see common.h.


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

To build the FileRepServiceExample sample
  1. Open the solution FileRepServiceExample.sln in Visual Studio. 
  2. Change the active solution platform to the desired platform in the 
     Configuration Manager found on the Build menu.
  3. On the Build menu, click Build. 

Running the Sample
==================

To run the FileRepServiceExample sample
  1. Run FileRepServiceExample by clicking Start Without Debugging on the Debug menu.

� Microsoft Corporation. All rights reserved.


