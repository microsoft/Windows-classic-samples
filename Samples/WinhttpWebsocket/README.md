WinHTTP WebSocket sample
========================

This sample demonstrates how to use the WinHTTP API to send and receive messages between a client and a server by using the WebSocket protocol.

The sample performs each step required to use the WebSocket connection. First, it creates the session, connection and request handles to open a HTTP connection. It then requests to upgrade the protocol from HTTP to the WebSocket protocol. The WebSocket handshake is performed by sending a request and receiving the appropriate response from the server. Data is then sent and received using the WebSocket protocol, and checks are made to ensure the complete message is transmitted. Finally, the connection is closed, and the close status and reason are confirmed.

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

For general information about WebSocket connections and how the protocol works, see the IETF's [WebSocket Protocol](http://go.microsoft.com/fwlink/p/?linkid=240293) documentation.

Related topics
--------------

[Windows HTTP Services (WinHTTP)](http://msdn.microsoft.com/en-us/library/windows/desktop/aa384273)

[**WinHttpSetOption**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa384114)

[**WinHttpWebSocketCompleteUpgrade**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh707326)

[**WinHttpWebSocketSend**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh707329)

[**WinHttpWebSocketReceive**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh707328)

[**WinHttpWebSocketClose**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh707325)

[**WinHttpWebSocketQueryCloseStatus**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh707327)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

To build the sample using Visual Studio:

1.  Open Windows Explorer and navigate to the **\\cpp** directory.
2.  Double-click the icon for the **WinhttpWebsocket.sln** file to open the file in Visual Studio.
3.  In the **Build** menu, select **Build Solution**. The application will be built in the default **\\Debug** or **\\Release** directory.

Run the sample
--------------

This sample requires that a web server that supports WebSockets is available for the app to access for sending and receiving data. The web server must be started before the app is run. The web server must also have a *WinHttpWebSocketSample* path available. The sample includes a PowerShell script that will install IIS on the local computer, create the *WinHttpWebSocketSample* folder on the server, and copy a file to this folder.

The easiest way to run the sample is to use the provided PowerShell scripts. Browse to the *Server* folder in your sample folder to setup and start the web server for WebSockets. There are two options possible.

-   Start PowerShell elevated (Run as administrator) and run the following command:

    **.\\SetupScript.ps1**

    Note that you may also need to change script execution policy.

-   Start an elevated Command Prompt (Run as administrator) and run following command:

    **PowerShell.exe -ExecutionPolicy Unrestricted -File SetupServer.ps1**

When the web server is not needed anymore, please browse to the *Server* folder in your sample folder and run one of the following:

-   Start PowerShell elevated (Run as administrator) and run the following command:

    **.\\RemoveScript.ps1**

    Note that you may also need to change script execution policy.

-   Start an elevated Command Prompt (Run as administrator) and run following command:

    **PowerShell.exe -ExecutionPolicy Unrestricted -File RemoveScript.ps1**

The sample can run using any web server that supports WebSockets, not only the one provided with the sample. In this case, running the previous scripts are not required. However, this requires some special configuration of the server to create the *WinHttpWebSocketSample* folder and copy a file to this folder. The sample must also be updated if run against a non-localhost web server:

To configure the sample for use with a different web server:

Copy the *Server\\webSite* directory to the *WinHttpWebSocketSample* folder on the web server and configure the server to support WebSockets.

The target server name should be updated in the sources. This is changed by editing the *WinhttpWebsocket.cpp* source file so that the *pcwszServerName* value that contains the hostname or IP address of the web server is substituted for localhost.

**Note**  IIS is not available on ARM builds. Instead, set up the web server on a separate 64-bit or 32-bit computer and follow the steps for using the sample against a non-localhost web server.

To run the sample:

-   Run **WinhttpWebsocket** by clicking **Start Without Debugging** on the **Debug** menu.

