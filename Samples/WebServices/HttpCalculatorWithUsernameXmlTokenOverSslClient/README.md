Windows Web Services HTTP calculator with XML token over SSL client sample
==========================================================================

This sample demonstrates how to use the [Windows Web Services API](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430435) to implement an HTTP client that uses the service proxy to talk to a calculator service with an XML security token over SSL mixed-mode security. In this setup, the transport connection is protected (signed, encrypted) by SSL which also provides server authentication. Client authentication is provided by a WS-Security username/password pair that is used as an XML security token by the example.

**Note**  This client side example uses the [**WS\_XML\_TOKEN\_MESSAGE\_SECURITY\_BINDING**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd323568). An equivalent way of doing the same client side security using the [**WS\_USERNAME\_MESSAGE\_SECURITY\_BINDING**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd323497) is illustrated by the example **Windows Web Services HTTP calculator with username over SSL client sample**. A matching server side for both of these client side examples is provided by the example **Windows Web Services HTTP calculator with username over SSL service**.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related topics
--------------

[**WsCloseServiceProxy**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430490)

[**WsCreateHeap**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430499)

[**WsCreateReader**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430504)

[**WsCreateServiceProxy**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430507)

[**WsCreateXmlSecurityToken**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430511)

[**WsFreeHeap**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430527)

[**WsFreeReader**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430531)

[**WsFreeServiceProxy**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430534)

[**WsFreeSecurityToken**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430532)

[**WsGetErrorProperty**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430539)

[**WsGetErrorString**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430540)

[**WsOpenServiceProxy**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430577)

[**WsReadType**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430601)

[**WsSetInput**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430631)

Related technologies
--------------------

[Windows Web Services API](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430435)

Operating system requirements
-----------------------------

Client

Windows 7

Server

Windows Server 2008 R2

Build the sample
----------------

To build this sample, open the CPP project solution (.sln) file within Visual Studio 2013 for Windows 8.1 (any SKU) or later versions of Visual Studio and Windows. Press F7 (or F6 for Visual Studio 2013) or go to Build-\>Build Solution from the top menu after the sample has loaded. The sample will be built in the default \\Debug or Release directory.

Run the sample
--------------

To run this sample after building it, press F5 (run with debugging enabled) or Ctrl-F5 (run without debugging enabled) from Visual Studio for Windows 8.1 (any SKU) or later versions of Visual Studio and Windows. (Or select the corresponding options from the Debug menu.)

