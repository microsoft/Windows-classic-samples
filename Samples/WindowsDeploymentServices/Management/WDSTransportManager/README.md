Windows Deployment Services transport manager sample
====================================================

Demonstrates how to use the Windows Deployment Services (WDS) Transport Management API, sometimes referred to as "WdsTptMgmt", to manage a WDS Transport Server.The API allows applications to manage and use the multicast features provided by a WDS Transport Server and enables download of any type of data by custom client applications.

-   The "WdsTptMgmt" API enables applications to manage and use the multicast features provided by a WDS Transport Server.
-   You can use the "WdsTptMgmt" to enable a custom client application to download any type of data.
-   This sample application can enable or disable WDS transport services and add or remove a transport namespace.

The WDS Transport Management API is a part of the overall [Windows Deployment Services](http://msdn.microsoft.com/en-us/library/windows/desktop/dd379586) Transport platform. It enables developers transmit any type of data using multicast transport and have complete control over server configuration, the data being for download, and the individual transport sessions and clients.

WdsTptMgmt provides programmatic support for the management of a local or remote WDS Transport Server. Management features are provided over a set interfaces. Setup management interfaces provide server information and support the registration of custom content providers. Configuration management interfaces control WDS Transport services and configure transport settings. This enables applications to enable and start services and to configure IP address ranges, port ranges, and diagnostic settings. Namespace management interfaces enable applications to add, remove, or query a registered transport namespace and manage active sessions and clients under a given namespace.

When you download this sample you will also receive a README.txt file.

Related topics
--------------

[Windows Deployment Services Transport Functions](http://msdn.microsoft.com/en-us/library/windows/desktop/bb394781)

[Windows Deployment Services multicast consumer sample](http://go.microsoft.com/fwlink/p/?linkid=254940)

[Windows Deployment Services multicast consumer sample](http://go.microsoft.com/fwlink/p/?linkid=254940)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

1.  Start Visual Studio and select **File \> Open \> Project/Solution**.
2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file titled WdsTransportManager.sln.
3.  Press F7 (or F6 for Visual Studio 2013) or use **Build \> Build Solution** to build the sample.

Run the sample
--------------

To debug the app and then run it, press F5 or use **Debug \> Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug \> Start Without Debugging**.

See the Readme.txt file provided with this sample for more information on how to run the Windows Deployment Services (WDS) Transport Management API sample.

Windows Deployment Services (WDS) enables the deployment of Windows operating systems using a network-based installation. You will also require the following to complete this sample.

-   Windows Server 2012 R2 with the Windows Deployment Services server role installed.
-   In order to manage this server using the "WdsTptMgmt" API, you need to have this API and associated management binaries installed on system running the client application.
-   The user running the application must be an administrator on the WDS server.

