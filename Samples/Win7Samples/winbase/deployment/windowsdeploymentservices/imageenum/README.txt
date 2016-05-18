========================================================================
    CONSOLE APPLICATION : ImageEnum Project Overview
========================================================================

Included is a sample application that uses the Windows Deployment Services 
Client library. The Windows Deployment Services Client library can be 
leveraged as part of a custom client application that takes the place of 
the Windows Deployment Services Client. This allows for a customized client 
solution that still leverages a Windows Deployment Services server as the 
back-end. 

The sample consists of the following files - 

ImgEnum.vcproj
    This is a Visual Studio file which contains information about the 
    version of Visual Studio that generated the file, and information 
    about the platforms, configurations, and project features.

imgenum.cpp
    This is the application's main source file.

imgenum.rc
    This is a listing of all of the Microsoft Windows resources that
    the program uses.  This file can be directly edited in Microsoft
    Developer Studio.

resource.h
   Required header file that defines helper data structures.

ImgEnum.sln
    This is the solution file for the ImgEnum sample generated using Visual 
    Studio. This is the solution file which should be loaded into Visual 
    Studio to generate the executable for this sample.

/////////////////////////////////////////////////////////////////////////////

Build process
==============
There are two ways to build this process: (1) Use Visual Studio, or (2) use makefile


Visual Studio Build Process
------------------------------
Open the solution file in Visual Studio. The project properties need to be 
modified to reflect your local environment. The include path needs to point to 
the Visual Studio include and Microsoft SDK include paths. The library path 
will point to the Microsoft SDK library path. There are three ways to do this. 
The first is to use the SDK tool to do the Registration. 
When you install the Microsoft SDK, under “Programs->Microsoft 
Windows SDK-> Visual Studio Registration”, there is a menu item for this
selection. Clicking this menu item will register the correct include and library 
directories for Visual Studio and your Visual Studio project will have the correct 
options to compile. The second is to directly modify the include paths and library 
paths in ImgEnum.vcproj. The third is to do so from Visual Studio. Go to the Project 
Properties dialog, and expand the C/C++ tab. Modify the "Additional Include 
Directories" to point to the correct Visual Studio and Microsoft SDK include 
paths. Next, expand the Linker tab, and modify the "Additional Library 
Directories" to point to the correct Microsoft SDK library path. Also make 
sure to update the Configuration Manager to include the correct platform. Once 
this is done, build the solution.


SDK Makefile Build Process
----------------------------
Use the SDK build environment window and type "nmake all" in your sample code 
directory.

/////////////////////////////////////////////////////////////////////////////

Implementing an Image Enumeration Solution
==========================================

In order to implement a full end-to-end solution using the image enumeration 
sample you will need:

1. A Windows server with the Windows Deployment Services server role 
   installed 
2. A Windows 7 Windows Pre-Installation Environment (Windows PE) image in 
   Windows Imaging (WIM) format that contains the Windows Vista setup.exe 
   and associated binaries. By default, such an image exists as boot.wim in
   the \Sources directory of the Windows Vista media. 

This sample requires the following .dlls to be copied from the Windows 7 
media \sources directory to the sample directory in order to run:

Wdsclientapi.dll
Wdsimage.dll
Wdscsl.dll
Wdstptc.dll

A custom scenario might look as follows:
a. A client machine boots into a version of Windows 7 that contains the 
   Windows 7 setup binaries. The client boot can be from any media – over 
   the network via PXE (this is the standard Windows Deployment Services 
   case), CD / DVD, hard drive, etc… 
b. A custom application is invoked. This application has the following 
   functionality – the application shows a customized UI.
c. The application detects the machine’s MAC address and contacts a database 
   to acquire the correct unattended setup file 
d. The application uses the Windows Deployment Services Client library to 
   retrieve a list of available images stored on a Windows Deployment Services
   server and displays the list of choices to the client using the custom UI. 
e. The application takes the user selected operating system image entry and 
   inserts the relative data into the unattended setup file. 
f. The application invokes Windows 7 setup.exe in unattended mode using 
   the unattended setup file acquired and customized per above. 

/////////////////////////////////////////////////////////////////////////////

Understanding the Client Library 
================================

The Windows Deployment Services Client library includes two main pieces of 
functionality –

1. The ability to enumerate images that are stored on a Windows Deployment 
   Services server 
2. The ability to send client installation events that can be used for 
   reporting / monitoring purposes (e.g. client installation started, client
   installation finished, etc…) 
 
The provided sample only exercises the first set of functionality – enumerating
images. As input, the sample takes credentials and the name of a valid Windows 
Deployment Services server. As output, the sample will return the list of 
available Windows Imaging (WIM) files stored on the Windows Deployment Services
server. In the background, the sample application will establish a session with
the specified Windows Deployment Services server, authenticate using the 
supplied credentials, retrieve a list of available images, extract the listed 
properties from the image, and print the output as below.

/////////////////////////////////////////////////////////////////////////////

Sample Program Usage
======================
This sample demonstrates the image enumeration functionality for the Windows 
Deployment Services Client API.

Command: ImgEnum.exe <UserId@Domain> <Password> <WDS Server>

Example:
ImgEnum.exe someone@NWTRADERS Password1 MyWDSServer

Sample Output:
Name = [Windows 7 ULTIMATE]
Description = [Windows 7 Ultimate Edition]
Path = [Images\Image Group (1)\Windows_7_ULTIMATE-NZUUV.wim]
Index = [1]
Architecture = [x86]

The ‘Name’ is the image name as stored in the XML metadata of the .WIM file. 
The 'Description’ is the image description as stored in the XML metadata of 
the .WIM file. The ‘Path’ is the relative path from the folder shared as 
‘REMINST’ to the .WIM file containing the image definition. To construct a 
full UNC path, simply append the Windows Deployment Services server name and
the well-known share name to the path value – 
e.g. ‘Images\Image Group (1)\Windows_7_ULTIMATE-NZUUV.wim' 
becomes 
‘\\MyWDSServer\REMINST\Images\Image Group (1)\Windows_7_ULTIMATE-NZUUV.wim’.

The ‘Index’ is the unique identifier within the .WIM file that points to the 
location within the .WIM file (a .WIM file may contain one or more images) where 
the image is stored; the index is 1-based. 

The ‘Architecture’ is the architecture value as stored in the XML metadata of 
the .WIM file.

The information returned may be used to populate the appropriate sections of 
an unattended setup file. For example, the ‘Name’ and ‘Description’ values 
above can be used to populate the relevant <InstallImage> section of a 
WDSClientUnattend file. E.g.

<InstallImage>
          <ImageName>*******insert image name here******</ImageName>
          <ImageGroup>*******insert image group here******</ImageGroup>
</InstallImage>


Alternatively, the ‘Path’ and ‘Index’ values can be used to populate the 
<InstallFrom> section of a regular setup.exe unattend file. E.g.

<InstallFrom>
          <Path>\\MyWDSServer\REMINST\Images\Image Group (1)\Windows_7_ULTIMATE-NZUUV.wim</Path>
          <MetaData>/Image/Index</MetaData>
          <Value>1</Value>
</InstallFrom>

/////////////////////////////////////////////////////////////////////////////

Windows Deployment Services Client API Logging
==============================================

The logging functionality of the Windows Deployment Services client library 
allows installation progress events to be sent via the client to the Windows 
Deployment Services server where the events are logged on the client’s behalf. 

On Windows Server 2003, the Windows Deployment Services server will log the 
client events to the debug trace log of the WDSServer service. The log file 
is located at ‘%windir%\tracing\wdsserver.log’ (once debug logging is enabled). 
Debug logging may be enabled by the following registry setting: 
‘HKLM\Software\Microsoft\Tracing\WDSServer, EnableFileTracing = 1’. 

On Windows Server 2008 and Windows Server 2008 R2, the Windows Deployment Services server
will log the client events to an application specific event log viewable 
through eventvwr.exe as well as the debug trace log (if enabled). In either 
case, Windows Deployment Services Client logging must be enabled globally on 
the server in order for the server to capture these events. This task may be 
accomplished either through the MMC or by running the command 
‘WDSUTIL.exe /set-server /WDSClientLogging’. Logging must be enabled (set to ‘yes’) 
and the logging level must match that being sent by the Windows Deployment Services 
Client library. For example, if your custom code is logging an event set to 
level 2 = WDS_LOG_LEVEL_WARNING, the Windows Deployment Services Server must 
have a /LoggingLevel value of ‘Warnings’ or ‘Info’ in order for the event 
to be recorded.

Note: This sample is compatible with Windows Vista too.

/////////////////////////////////////////////////////////////////////////////