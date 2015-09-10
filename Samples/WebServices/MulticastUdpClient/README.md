Windows Web Services multicast UDP client sample
================================================

This sample demonstrates how to use the [Windows Web Services API](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430435) to implement a UDP client that multicasts a request and then waits for reply messages from servers.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related topics
--------------

[**WsAbortChannel**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430474)

[**WsAcceptChannel**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430478)

[**WsAddressMessage**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430482)

[**WsCloseChannel**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430487)

[**WsCreateChannel**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430495)

[**WsCreateError**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430497)

[**WsCreateHeap**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430499)

[**WsCreateMessageForChannel**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430502)

[**WsFreeChannel**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430525)

[**WsFreeError**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430526)

[**WsFreeHeap**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430527)

[**WsFreeMessage**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430529)

[**WsGetErrorProperty**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430539)

[**WsGetErrorString**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430540)

[**WsGetHeader**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430543)

[**WsInitializeMessage**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430568)

[**WsOpenChannel**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430574)

[**WsReadBody**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430583)

[**WsReadMessageEnd**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430593)

[**WsReadMessageStart**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430594)

[**WsResetHeap**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430615)

[**WsResetMessage**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430617)

[**WsSetChannelProperty**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430626)

[**WsSetHeader**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430630)

[**WsWriteBody**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430648)

[**WsWriteMessageStart**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430660)

[**WsWriteMessageEnd**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430659)

[**WsXmlStringEquals**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430673)

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

To build this sample, open the CPP project solution (.sln) file within Microsoft Visual Studio Express 2013 for Windows 8.1 or later versions of Visual Studio and Windows (any SKU). Press F7 (or F6 for Visual Studio 2013) or go to Build-\>Build Solution from the top menu after the sample has loaded. The sample will be built in the default \\Debug or Release directory.

Run the sample
--------------

To run this sample after building it, press F5 (run with debugging enabled) or Ctrl-F5 (run without debugging enabled) from Visual Studio Express 2013 for Windows 8.1 or later versions of Visual Studio and Windows (any SKU). (Or select the corresponding options from the Debug menu.)

