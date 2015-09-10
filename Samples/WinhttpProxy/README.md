WinHTTP proxy sample
====================

This sample demonstrates how to use the WinHTTP API to determine the proxy for a particular URL. It uses the core functionality provided in WinHTTP for querying the proxy settings.

Both [**WinHttpGetProxyForUrl**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa384097) and [**WinHttpGetProxyForUrlEx**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh405356) are used in this sample. These functions implement the Web Proxy Auto-Discovery (WPAD) protocol for automatically configuring the proxy settings for an HTTP request. The WPAD protocol downloads a Proxy Auto-Configuration (PAC) file, which is a script that identifies the proxy server to use for a given target URL. PAC files are typically deployed by the IT department within a corporate network environment. These functions can automatically discover the location of the PAC file on the local network.

This sample can be extended as needed for your application, using the proxy code from this sample as a starting point, to add additional functionality. For example, an application could add a per-URL proxy cache, awareness for network changes, a filter for bad proxies, or other desired functionality.

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related topics
--------------

[Windows HTTP Services (WinHTTP)](http://msdn.microsoft.com/en-us/library/windows/desktop/aa384273)

[**WinHttpOpenRequest**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa384099)

[**WinHttpCreateProxyResolver**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh405355)

[**WinHttpFreeProxyResult**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh707321)

[**WinHttpGetProxyResult**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh707322)

[**WinHttpGetProxyForUrlEx**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh405356)

[**WINHTTP\_PROXY\_INFO**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa383912)

Related technologies
--------------------

[Windows HTTP Services (WinHTTP)](http://msdn.microsoft.com/en-us/library/windows/desktop/aa384273)

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
2.  Double-click the icon for the **WinhttpProxySample.sln** file to open the file in Visual Studio.
3.  In the **Build** menu, select **Build Solution**. The application will be built in the default **\\Debug** or **\\Release** directory.

Run the sample
--------------

To run the sample:

1.  Navigate to the directory that contains the new executable, using the command prompt.
2.  Type **WinhttpProxySample.exe \<url\>** at the command prompt.

