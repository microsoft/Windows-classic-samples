========================================================================
    CONSOLE APPLICATION : Consumer Project Overview
========================================================================

Included is a sample application that uses the Windows Deployment Services
Multicast Client library to implement a simple multicast consumer.  A multicast
consumer can be used with a custom multicast provider to transport arbitrary
user defined data over multicast.

The sample consists of the following files -

consumer.vcxproj
    This is a Visual Studio file which contains information about the
    version of Visual Studio that generated the file, and information
    about the platforms, configurations, and project features.

main.cpp
    This is the application's main source file.

wdsscon.rc
    This is a listing of all of the Microsoft Windows resources that
    the program uses.  This file can be directly edited in Microsoft
    Visual Studio.

consumer.sln
    This is the solution file for the consumer sample generated using Visual
    Studio. This is the solution file which should be loaded into Visual
    Studio to generate the executable for this sample.

/////////////////////////////////////////////////////////////////////////////

Understanding the Multicast Solution
====================================

A content consumer is just one part of a pair of components that must be
implemented in order to transmit custom data over multicast.  The other piece
that is required is a content provider that encodes the data to be transmitted
into a sequence of data blocks.  It is the content consumer's responsibility
to interpret these blocks of data.

The provided sample implements a simple consumer that merely dumps that data it
receives to a file.  An application that wishes to interpret the data received
over multicast differently needs to reimplement the ReceiveContentsCallback
function.

/////////////////////////////////////////////////////////////////////////////

Implementing a Multicast Solution
=================================

In order to implement a full end-to-end solution using the multicast consumer
sample you will need:

1. Build/Choose a content provider DLL that encodes the data in a format this
   consumer application will understand. For example, you can use the content provider
   sample provided with the Windows Deployment Services Transport SDK to accomplish
   this.
2. Install the content provider DLL on the WDS Transport Server and register it.
   Please refer to the content provider sample ReadMe.txt file for more
   information.
3. Create a WDS Transport "Namespace" on the server that defines the content
   provider to be used (e.g. the sample content provider) and the folder containing
   the file(s) to be transferred over multicast. This can be accomplished via the
   Windows Deployment Services Transport Management (WdsTptMgmt) API.
4. Use this consumer application to download files exposed by the above namespace.

/////////////////////////////////////////////////////////////////////////////

Sample Program Usage
======================
This sample demonstrates connecting to a multicast session, receiving a file,
and writing the file to disk.

To run the sample, simply follow the usage instructions below.

Consumer.exe <server> <namespace> <remote file> <local file>

    server:      The hostname or ip address of the multicast server.
    namespace:   A user defined namespace on the server.
    remote file: The name of the content exposed by the namespace on the server.
    local file:  The local file name to which to save the file.

    Example:
    consumer.exe MyWdsServer Namespace1 DataFile.dat c:\LocalFile.dat
