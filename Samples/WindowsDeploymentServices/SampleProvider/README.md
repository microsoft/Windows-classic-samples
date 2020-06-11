Windows Deployment Services provider sample
===========================================

Demonstrates how to create a custom PXE provider DLL that can replace or run in conjunction with the standard WDSDCPXE PXE provider on a Windows Deployment Services (WDS) server.

[Windows Deployment Services](http://msdn.microsoft.com/en-us/library/windows/desktop/dd379586) (WDS) enables the deployment of Windows operating systems. You can use WDS to set up new clients with a network-based installation without requiring that administrators visit each computer or install directly from CD or DVD media. The PXE Server contains the core networking capability of the server solution. The PXE Server supports a plug-in interface. Plug-ins are also known as "PXE Providers". This provider model allows for custom PXE solutions to be developed while leveraging the same core PXE Server networking code base.

-   Plug-ins known as "PXE Providers" enable you to develop custom solutions that leverage the core networking code base of the WDS PXE Server implementation. This sample provider allows you to create a DLL that may replace or run in conjunction with the existing PXE Provider, BINL, on a Windows Deployment Services server.
-   This provider sample stores data in a text file rather than Active Directory.
-   This provider sample adds a Boot option for a BCD file to the DHCP reply packet sent out by the server.

A simple walkthrough for using the sample filter provider consists of the following. First, compile the sample code into a DLL. Second, create an .ini file. Third, register the sample provider in the ordered provider list. Fourth, boot a sample client using PXE. Any requests not filtered by a preceding provider will then be passed to the next registered provider in the ordered list.

When you download this sample you will also receive a README.txt file.

Related topics
--------------

[Using the Windows Deployment Services Server API](http://msdn.microsoft.com/en-us/library/windows/desktop/bb530732)

[Windows Deployment Services provider sample](http://go.microsoft.com/fwlink/p/?linkid=254936)

[Windows Deployment Services image enumeration sample](http://go.microsoft.com/fwlink/p/?linkid=254937)

[Windows Deployment Services transport manager sample](http://go.microsoft.com/fwlink/p/?linkid=254942)

Operating system requirements
-----------------------------

Client

None supported

Server

Windows Server 2012 R2

Build the sample
----------------

1.  Start Visual Studio and select **File \> Open \> Project/Solution**.
2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file titled SampProv.sln.
3.  Press F7 (or F6 for Visual Studio 2013) or use **Build \> Build Solution** to build the sample.

Run the sample
--------------

To debug the app and then run it, press F5 or use **Debug \> Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug \> Start Without Debugging**.

For detailed information on how to run the Windows Deployment Services PXE provider sample see the Readme.txt file provided with this sample.

Windows Deployment Services (WDS) enables the deployment of Windows operating systems using a network-based installation. You also need the following to complete this sample.

-   Install the Windows Deployment Services server role on Windows Server 2012 R2.
-   Initialize the Windows Deployment Services server.
-   Add a boot image (Boot.wim) to the server. This is a Windows PE image that a client computer can boot into to install the operating system. The boot image (Boot.wim) can be found on the Windows product DVD.

