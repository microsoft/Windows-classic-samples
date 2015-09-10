========================================================================
    CONSOLE APPLICATION : WdsTransportManager Project Overview
========================================================================

This sample application illustrates using the Windows Deployment Services
(WDS) Transport Management API, sometimes referred to as WdsTptMgmt, to
manage a WDS Transport Server. The API allows 3rd party applications
to manage and use the multicast features provided by a WDS Transport
Server in order to allow the download of any type of data by custom
client applications.

The files contained in this sample are described below.

WdsTransportManager.sln
    This is the solution file for the WdsTransportManager sample generated
    using Visual Studio. This is the solution file which should be loaded
    into Visual Studio to generate the executable for this sample.

WdsTransportManager.vcxproj
    This is the main project file for VC++ projects generated using an
    Application Wizard. It contains information about the version of Visual C++
    that generated the file, and information about the platforms,
    configurations, and project features selected with the Application Wizard.

WdsTransportManager.cpp
    This is the main application source file. It contains the main function
    and associated helper functions that illustrate using the WdsTptMgmt
    API.

WdsTransportManager.h
    This is the main application header file containing type definitions,
    utility macros and similar items.

Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named WdsTransportManager.pch and a precompiled types file named StdAfx.obj.

ReadMe.txt
    This ReadMe file.

/////////////////////////////////////////////////////////////////////////////

The Windows Deployment Services Transport Management API
========================================================

The WDS Transport Management API is a part of the overall WDS Transport
platform which allows any software developer to develop a complete solution
to transmit any type of data over a reliable multicast transport and have
complete control over server configuration, the data being offered for
download, and the individual transport sessions and clients.

WdsTptMgmt is designed to provide full programmatic support for the
management tasks of a local or remote WDS Transport Server. Management
features are provided over a set of COM interfaces which include:

1. Setup Management:
    a. Provides information on the server, such as host OS version.
    b. Allows applications to register/deregister custom content providers.

2. Configuration Management:
    a. Allows applications to control the WDS Transport services, such as
       enabling, disabling, or starting the services.
    b. Allows applications to configure various transport settings, such
       as IP address ranges, port ranges, and diagnostic settings.

3. Namespace Management:
    a. Allows applications to add or remove a registered transport namespace
       or query information on registered namespaces. Transport namespaces
       define what content is being offered for download, what content provider
       is responsible for preparing the data for transmission, and transmission
       start parameters, such as an absolute time at which transmission would
       begin.
    b. Allows applications to query and manage active sessions and clients
       under a given namespace, including support for terminating a session
       or disconnecting a particular client.

For more information, please refer to the Windows SDK documentation.

/////////////////////////////////////////////////////////////////////////////

Sample Program Usage
====================
This sample demonstrates using WdsTptMgmt to enable/disable WDS transport
services and add/remove a transport namespace.

To run the sample, simply follow the usage instructions below.

WdsTptMgr.exe <WDS Server> <Action> [ <Namespace Name> <Folder Path>
       <Minimum Clients> ]

       where:

       <WDS Server>: Name of a WDS Transport Server
       <Action>: One of the values EnableServer, DisableServer,
                 AddNamespace, RemoveNamespace
       <Namespace Name>: The name of the namespace to add/remove.
       <Folder Path>: Full path to the folder to be offered for
                      download in a namespace that is being added.
       <Minimum Clients>: The minimum number of clients to auto-start
                          a namespace that is being added.

At the end of execution, WdsTptMgr.exe will print a final result that
either indicates success or failure (accompanied by an HRESULT error code).

NOTE:
The user running the application has to be an administrator on the
server specified in the <WDS Server> argument.

/////////////////////////////////////////////////////////////////////////////
