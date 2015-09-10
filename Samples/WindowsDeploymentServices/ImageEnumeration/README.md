Windows Deployment Services image enumeration sample
====================================================

Demonstrates how to use the Windows Deployment Services (WDS) Client API to enumerate images that are stored on a WDS server.

[Windows Deployment Services](http://msdn.microsoft.com/en-us/library/windows/desktop/dd379586) (WDS) enables the deployment of Windows operating systems. You can use WDS to set up new clients with a network-based installation without requiring that administrators visit each computer or install directly from CD or DVD media. The Windows Deployment Services Client library can be leveraged as part of a custom client application that takes the place of the Windows Deployment Services Client. This allows for a customized client solution that still leverages a Windows Deployment Services server as the back-end.

-   The application demonstrates how a custom client application can use the Windows Deployment Services Client library to take the place of the WDS client.
-   You can use the WDS client library to develop custom client applications that use a WDS server. This allows for a customized client solution that still leverages a Windows Deployment Services server as the back-end.
-   This sample takes credentials and the name of a WDS server and enumerates the available Windows Imaging (WIM) files stored on the server.

The functionality included in the Windows Deployment Services Client library is capable of enumerating images stored on a Windows Deployment Services server. The sample takes the credentials and the name of a valid Windows Deployment Services server and returns a list of available Windows Imaging (WIM) files stored on the Windows Deployment Services server. In the background, the sample application will establish a session with the specified Windows Deployment Services server, authenticate using the supplied credentials, retrieve a list of available images, extract the listed properties from the image, and print the output. This particular sample does not make use of the capability to send client installation events to report or monitor the start and finish of a client installation.

When you download this sample you will also receive a README.txt file.

Related topics
--------------

[Using the Windows Deployment Services Client API](http://msdn.microsoft.com/en-us/library/windows/desktop/bb530731)

[Windows Deployment Services filter provider sample](http://go.microsoft.com/fwlink/p/?linkid=254935)

[Windows Deployment Services multicast consumer sample](http://go.microsoft.com/fwlink/p/?linkid=254940)

[Windows Deployment Services multicast provider sample](http://go.microsoft.com/fwlink/p/?linkid=254941)

[Windows Deployment Services provider sample](http://go.microsoft.com/fwlink/p/?linkid=254936)

[Windows Deployment Services transport manager sample](http://go.microsoft.com/fwlink/p/?linkid=254942)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

1.  Start Visual Studio and select **File \> Open \> Project/Solution**.
2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file titled ImgEnum.sln.
3.  Press F7 (or F6 for Visual Studio 2013) or use **Build \> Build Solution** to build the sample.

Run the sample
--------------

To debug the app and then run it, press F5 or use **Debug \> Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug \> Start Without Debugging**.

For detailed information on how to run the Windows Deployment Services image enumeration sample see the Readme.txt file provided with this sample.

Windows Deployment Services (WDS) enables the deployment of Windows operating systems using a network-based installation. You will also require the following to complete this sample.

-   Windows Server 2012 R2 with the Windows Deployment Services server role installed.
-   A Windows Pre-Installation Environment (Windows PE) image in the Windows Imaging (WIM) format that contains the setup.exe and associated binaries.
-   Wdsclientapi.dll, Wdsimage.dll, Wdscsl.dll and Wdstptc.dll copied into the sample directory.

