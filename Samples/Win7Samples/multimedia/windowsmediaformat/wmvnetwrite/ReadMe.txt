========================================================================
       WIN32 Console Application : WMVNetWrite 
========================================================================

Shows how a Windows Media file is streamed across the Internet. The sample requires a port to be specified, and then the file can be played using a player. 

USAGE:

wmvnetwrite -i <infile> [-p <portnum>] [-c <maxclient>] [-s <server URL>]
        infile  = Input WMV file name
        portnum = Port number (for incoming connections)
        maxclient = Maximum clients allowed to connect
        server URL = URL of Server for push distribution


  For example, in order to stream the file from the host machine and to play this stream on a client machine,
the following should be run on the host machine:

WMVNetWrite -i c:\filename.asf -p 8080

  And then the following URL should be opened with the Windows Media Player on the client machine:

http://hostmachinename:8080  (where hostmachinename is the name of the computer running WMVNetWrite.exe).


HOW TO BUILD:

  In order to build the sample executable, open the project file WMVNetWrite.sln in Visual C++ and build
the project.


REMARKS:

  -This sample is not able to stream protected Windows Media files.
  -Push distribution to servers requiring authentication is not supported in this sample.


IMPORTANT INTERFACES AND METHODS DEMONSTRATED IN THIS SAMPLE:

  IWMReaderCallback, IWMReaderCallbackAdvanced and IWMRegisterCallback interfaces

  IWMWriter::BeginWriting
  IWMWriterAdvanced::WriteStreamSample
  IWMWriterAdvanced::RemoveSink
  IWMWriterAdvanced::RemoveSink

  IWMWriterNetworkSink::SetNetworkProtocol
  IWMWriterNetworkSink::Open
  IWMWriterNetworkSink::GetHostURL
  IWMWriterNetworkSink::SetMaximumClients
  IWMWriterNetworkSink::Connect
