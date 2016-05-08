========================================================================
    DYNAMIC LINK LIBRARY : Provider Project Overview
========================================================================

Included is a sample library that uses the Windows Deployment Services 
Multicast Server library to implement a simple multicast provider.  A multicast
provider can be used with a custom multicast consumer to transport arbitrary
user defined data over multicast.  

The sample consists of the following files - 

provider.vcproj
    This is a Visual Studio file which contains information about the 
    version of Visual Studio that generated the file, and information 
    about the platforms, configurations, and project features.

main.c
    This is the DLL's main source file.

wdscp.rc
    This is a listing of all of the Microsoft Windows resources that
    the program uses.  This file can be directly edited in Microsoft
    Visual Studio.

wdscp.def
    This defines all the functions exported by the provider library.

provider.sln
    This is the solution file for the provider sample generated using Visual 
    Studio. This is the solution file which should be loaded into Visual 
    Studio to generate the DLL for this sample.

/////////////////////////////////////////////////////////////////////////////

Build process
==============
There are two ways to build this process: (1) Use Visual Studio, or (2) Use VCBuild


Visual Studio Build Process
------------------------------
Open the solution file in Visual Studio. The project properties need to be 
modified to reflect your local environment. The include path needs to point to 
the Visual Studio include and Microsoft SDK include paths. The library path 
will point to the Microsoft SDK library path. There are three ways to do this. 
The first is to use the SDK tool to do the Registration.  When you install the 
Microsoft SDK, under “Programs->Microsoft Windows SDK-> Visual Studio Registration”, 
there is a menu item for this selection. Clicking this menu item will register the correct 
include and library directories for Visual Studio and your Visual Studio project will have 
the correct options to compile. The second is to directly modify the include paths and 
library paths in provider.vcproj. The third is to do so from Visual Studio. Go to the 
Project Properties dialog, and expand the C/C++ tab. Modify the "Additional Include 
Directories" to point to the correct Visual Studio and Microsoft SDK include 
paths. Next, expand the Linker tab, and modify the "Additional Library 
Directories" to point to the correct Microsoft SDK library path. Also make 
sure to update the Configuration Manager to include the correct platform. Once 
this is done, build the solution.


SDK VCBuild Process
-------------------
Use the SDK build environment window and type "VCBuild provider.vcproj"
in your sample code directory.

/////////////////////////////////////////////////////////////////////////////

Understanding the Multicast Solution 
====================================

A content provider is just one part of a pair of components that must be
implemented in order to transmit custom data over multicast.  The other piece 
that is required is a content consumer that decodes the sequence of data blocks
that it receives over multicast.  It is the content provider's responsibility
to encode the data the user is transmitting into a series of blocks that the
content consumer can decode.

The provided sample implements a simple provider that merely encodes a file on
the server into a series of data blocks.  

/////////////////////////////////////////////////////////////////////////////

Implementing a Multicast Solution
=================================

In order to implement a full end-to-end solution using the multicast provider 
sample you will need to:

1. Build the content provider DLL.
2. Install the content provider DLL on the WDS Transport Server and register it.
   This is a one-time process, after which the WDS Transport Server services
   need to be restarted. Registration can be accomplished by calling the
   IWdsTransportSetupManager::RegisterContentProvider method of the Windows
   Deployment Services Transport Management (WdsTptMgmt) API.
3. Create a WDS Transport "Namespace" on the server that defines this sample
   as the content provider and the folder containing the file(s) to be
   transferred over multicast. Again, this can be accomplished via the
   WdsTptMgmt API.
4. Use a consumer application to download files exposed by the above namespace.
   For example, the consumer application sample provided with the Windows
   Deployment Services Transport SDK can be used to accomplish this.

/////////////////////////////////////////////////////////////////////////////

