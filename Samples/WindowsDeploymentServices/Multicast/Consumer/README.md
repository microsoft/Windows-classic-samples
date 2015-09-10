Windows Deployment Services multicast consumer sample
=====================================================

Demonstrates how to use the [Windows Deployment Services](http://msdn.microsoft.com/en-us/library/windows/desktop/dd379586) (WDS) Multicast Client library to implement a multicast consumer.

-   A multicast consumer can be used with a multicast provider to transport user-defined data over multicast.
-   This multicast consumer sample demonstrates connecting to a multicast session, receiving a file from a multicast provider, and writing the file to disk.
-   An actual application would need to re-implement the **ReceiveContentsCallback** function of this sample to interpret the transmitted data.
-   You can use the [Windows Deployment Services multicast provider sample](http://go.microsoft.com/fwlink/p/?linkid=258652) to provide data in a format that can be understood by this sample consumer.

A content consumer is just one part of a pair of components that must be implemented in order to transmit custom data over multicast. The other piece that is required is a content provider that encodes the data to be transmitted into a sequence of data blocks. It is the content consumer's responsibility to interpret these blocks of data.

A simple walkthrough for using the multicast consumer sample consists of the following. First, choose a content provider DLL that encodes the data in a format this consumer application will understand, such as the [Windows Deployment Services multicast provider sample](http://go.microsoft.com/fwlink/p/?linkid=258652). Second, install the content provider DLL on the WDS Transport Server and register it. Third, use the Windows Deployment Services Transport Management (WdsTptMgmt) API to create a WDS Transport namespace on the server that specifies the sample content provider and the folder containing the files to be transferred over multicast. Finally, use the consumer application to download files exposed by the namespace.

When you download this sample you will also receive a README.txt file.

Related topics
--------------

[Windows Deployment Services Transport Functions](http://msdn.microsoft.com/en-us/library/windows/desktop/bb394781)

[Windows Deployment Services multicast provider sample](http://go.microsoft.com/fwlink/p/?linkid=258652)

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
2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file titled consumer.sln.
3.  Press F7 (or F6 for Visual Studio 2013) or use **Build \> Build Solution** to build the sample.

Run the sample
--------------

To debug the app and then run it, press F5 or use **Debug \> Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug \> Start Without Debugging**.

See the Readme.txt file provided with this sample for more information on how to run the multicast consumer sample.

A consumer is only one of the two parts required to transmit custom data over multicast. The other piece is a multicast provider that encodes the data into a sequence of data blocks into a format that can be received and understood by the consumer application. To implement a full solution, you can use the multicast consumer sample together with the [Windows Deployment Services multicast provider sample](http://go.microsoft.com/fwlink/p/?linkid=258652).

