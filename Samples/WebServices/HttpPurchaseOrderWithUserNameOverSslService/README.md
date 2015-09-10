Windows Web Services HTTP purchase order with username over SSL service sample
==============================================================================

This sample demonstrates how the [Windows Web Services API](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430435) can use the service host for hosting a PurchaseOrder service over HTTP, with username over SSL mixed-mode security. In this setup, the transport connection is protected (signed and encrypted) by SSL, which also provides server authentication. Client authentication is provided by a username/password pair in a WS-Security header in the message.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related topics
--------------

[**WS\_FAULT\_DETAIL\_DESCRIPTION**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd401878)

[**WS\_SERVICE\_CONTRACT**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd323418)

[**WS\_SSL\_TRANSPORT\_SECURITY\_BINDING**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd323441)

[**WS\_STRING**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd323447)

[**WS\_USERNAME\_MESSAGE\_SECURITY\_BINDING**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd323497)

[**WS\_XML\_STRING**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd323559)

[**WsAddErrorString**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430480)

[**WsCreateServiceHost**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430506)

[**WsGetMessageProperty**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430548)

[**WsGetOperationContextProperty**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430553)

[**WsOpenServiceHost**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430576)

[**WsSetFaultErrorDetail**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430628)

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

