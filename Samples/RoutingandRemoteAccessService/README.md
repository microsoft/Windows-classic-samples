Routing and Remote Access Service (RRAS) MPR API sample
=======================================================

Demonstrates how to use Windows 8.1 MPR APIs to set and retrieve configuration info from Routing and Remote Access Service (RRAS).

The sample demonstrates how to:

-   configure custom IPSec policies on a demand dial interface.
-   remove a custom IPSec configuration from a demand dial interface.
-   configure custom IPSec policies (to be applied on all the VPN connections) on a RRAS server.
-   remove custom a IPSec configuration from a RRAS server.
-   enumerate the VPN connections on a RRAS server.

The APIs demonstrated in this sample are:

-   [**MprAdminServerConnect**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa375840) - to establish a connection to a RRAS server for the purpose of administering the RRAS.
-   [**MprAdminServerDisconnect**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa375843) - to disconnect the connection made using [**MprAdminServerConnect**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa375840).
-   [**MprAdminInterfaceCreate**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa374573) - to create a demand dial interface on a specified RRAS server.
-   [**MprAdminInterfaceDeviceGetInfo**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa374575) - to retrieve info for a specified interface on a server.
-   [**MprAdminInterfaceGetHandle**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa374581) - to retrieve a handle to a specified interface for administering that interface.
-   MprAdminInterfaceSetCustomInfoEx - to set tunnel-specific custom configuration info for a specified demand dial interface on a server.
-   MprAdminInterfaceGetCustomInfoEx - to retrieve tunnel-specific custom configuration info for a specified demand dial interface on a server.
-   [**MprAdminBufferFree**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa374557) - to release the memory buffers returned by the MprAdmin\* APIs.
-   [**MprAdminServerGetInfoEx**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd408070) - to retrieve the tunnel-specific configuration of the RRAS server.
-   [**MprAdminServerSetInfoEx**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd408071) - to set the tunnel-specific configuration on a specified RRAS server.
-   [**MprAdminConnectionEnum**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa374559) - to enumerate all active VPN connections on a specified RRAS server.
-   [**MprConfigServerConnect**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa375874) - to connect to a RRAS server to be configured.
-   [**MprConfigServerDisconnect**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa375875) - to disconnect a connection made using [**MprConfigServerConnect**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa375874).
-   [**MprConfigInterfaceCreate**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa375860) - to create a router interface in the specified router configuration.
-   [**MprConfigInterfaceGetHandle**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa375864) - to retrieve a handle to the specified interface's configuration in a router configuration.
-   MprConfigInterfaceSetCustomInfoEx - to set the custom IKEv2 policy configuration for a specified interface in a router configuration.
-   MprConfigInterfaceGetCustomInfoEx - to retrieve the custom IKEv2 policy configuration for a specified interface in a router configuration.
-   [**MprConfigBufferFree**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa375855) - to free the buffers allocated by calls to the MprConfig\* APIs.
-   [**MprConfigServerGetInfoEx**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd408076) - to retrieve tunnel-specific custom configuration information for a specified demand dial interface on a server.
-   [**MprConfigServerSetInfoEx**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd408077) - to set tunnel-specific custom configuration for a specified demand dial interface on a server.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related technologies
--------------------

[Routing and Remote Access Service](http://msdn.microsoft.com/en-us/library/windows/desktop/bb545679)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

To build this sample, open the CPP project solution (.sln) file within Visual Studio Express 2013 for Windows 8.1 or later versions of Visual Studio and Windows (any SKU). Press F7 (or F6 for Visual Studio 2013) or go to Build-\>Build Solution from the top menu after the sample has loaded. The sample will be built in the default \\Debug or Release directory.

Run the sample
--------------

To run this sample after building it, press F5 (run with debugging enabled) or Ctrl-F5 (run without debugging enabled) from Visual Studio Express 2013 for Windows 8.1 or later versions of Visual Studio and Windows (any SKU). (Or select the corresponding options from the Debug menu.)

In order to see the enumerated VPN connections using this sample, you must have VPN clients connected to the RRAS server. Otherwise, this sample would always show "No VPN connections are available".

