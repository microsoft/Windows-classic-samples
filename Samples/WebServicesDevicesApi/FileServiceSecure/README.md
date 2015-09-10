WSD secure file services sample
===============================



This sample demonstrates how to use a selected set of security features in the Web Services for Devices API.



This sample is a modification to the **WSD file services sample**, which introduces security features. The selected set of security features includes the following:

-   Server authentication over TLS using X509 certificates. The client authenticates the server using the server certificate thumb-print (certificate hash).
-   Client authentication using either NTLM/Negotiate http authentication scheme or X509 client certificate present in the current user store.

This sample shows how to use a user token, which is a handle containing the credentials of the client. The service uses this token to impersonate the user and then execute any of the code paths through that user.

**Warning**  This sample requires Microsoft Visual Studio 2013 or a later version (any SKU) and will not compile in Microsoft Visual Studio Express 2013 for Windows.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related topics
--------------

Web Services for Devices API

[Configuring WSDAPI Applications to Use a Secure Channel](http://msdn.microsoft.com/en-us/library/windows/desktop/aa823078)

Related technologies
--------------------

Web Services for Devices API

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  2. Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file.

3.  Press F6 or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

The client and service applications can run on the same computer or a different one. Be aware that secure channel configurations are required prior to using the client and service applications. For more information, see [Configuring WSDAPI Applications to Use a Secure Channel](http://msdn.microsoft.com/en-us/library/windows/desktop/aa823078).

To run the service type:

1.  Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
2.  Type **FileServiceSecureService.exe** at the command prompt, or double-click the icon for **FileServiceSecureService** to launch it from Windows Explorer.

To run the client type:

1.  Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
2.  Type **FileClientSecureService.exe** at the command prompt, or double-click the icon for **FileClientSecureService** to launch it from Windows Explorer.

