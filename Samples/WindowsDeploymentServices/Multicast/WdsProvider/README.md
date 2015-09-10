Windows Deployment Services multicast provider sample
=====================================================

Demonstrates how to use the [Windows Deployment Services](http://msdn.microsoft.com/en-us/library/windows/desktop/dd379586) (WDS) Multicast Client library to implement a multicast provider.

-   A multicast provider can be used with a multicast consumer to transport user-defined data over multicast.
-   This multicast provider sample encodes a file on the server into a series of data blocks in a format that can be understood by the [Windows Deployment Services multicast consumer sample](http://go.microsoft.com/fwlink/p/?linkid=254940).
-   You can re-implement the **ReceiveContentsCallback** function of the multicast consumer sample to connect to a multicast session, receive data from the multicast provider sample, and interpret the transmitted data.

A content provider is just one part of a pair of components that must be implemented in order to transmit custom data over multicast. The other piece that is required is a content consumer that decodes the sequence of data blocks that it receives over multicast, such as the [Windows Deployment Services multicast consumer sample](http://go.microsoft.com/fwlink/p/?linkid=254940). It is the content provider's responsibility to encode the data the user is transmitting into a series of blocks that the content consumer can decode.

A simple walkthrough for using the multicast provider sample consists of the following. First, build the content provider DLL. Second, install the content provider DLL on the WDS Transport Server and register it. Third, use the Windows Deployment Services Transport Management (WdsTptMgmt) API to create a WDS Transport namespace on the server that specifies the sample content provider and the folder containing the files to be transferred over multicast. Finally, use a consumer application to download files exposed by the namespace.

When you download this sample you will also receive a README.txt file.

Related topics
--------------

[Windows Deployment Services Transport Functions](http://msdn.microsoft.com/en-us/library/windows/desktop/bb394781)

[Windows Deployment Services multicast consumer sample](http://go.microsoft.com/fwlink/p/?linkid=254940)

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
2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file titled provider.sln.
3.  Press F7 (or F6 for Visual Studio 2013) or use **Build \> Build Solution** to build the sample.

Run the sample
--------------

To debug the app and then run it, press F5 or use **Debug \> Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug \> Start Without Debugging**.

See the Readme.txt file provided with this sample for more information on how to run the multicast provider sample.

A multicast provider is only one of the two parts required to transmit custom data over multicast. The multicast provider encodes the data being transmitted into a sequence of data blocks. A content consumer application is required to receive and interpret this data. The multicast provider DLL must encode the data in a format that can be understood by the consumer application. You can use the [Windows Deployment Services multicast consumer sample](http://go.microsoft.com/fwlink/p/?linkid=254940) to receive data encoded by this multicast provider sample.

