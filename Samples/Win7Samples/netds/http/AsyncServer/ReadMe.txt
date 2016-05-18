SUMMARY
=======
This sample demonstrates how to create a simple HTTP server using the HTTP API, v2. It does this using the system thread pool. 
Threads within the thread pool receives I/O completions from the specified HTTPAPI request queue. They process these by calling the 
callback function according to the I/O context. As an example, we send back an HTTP response to the specified HTTP request. If the request
was valid, the response will include the content of a file as the entity body.


Security Note 
=============

 This sample is provided for educational purpose only to demonstrate how to use Windows 
 HTTP API, v2. It is not intended to be used without modifications in a production 
 environment and it has not been tested in a production environment. Microsoft assumes 
 no liability for incidental or consequential damages should the sample code be used for 
 purposes other than as intended.

USAGE
=====
This sample includes Microsoft Visual Studio .NET project files. To create AsynchronousHTTPServerApp.exe,
load AsynchronousHTTPServerApp.sln and build the project.

To run the sample:
C:>AsynchronousHTTPServerApp.exe <Url> <ServerDirectory>

where:

<Url>             is the Url base this sample will listen for. The URL must not have a query element.
<ServerDirectory> is the local directory to map incoming requested Url to. Note that this path should not include a trailing slash.

Example:
C:>AsynchronousHTTPServerApp.exe http://*:80/ C:\httpsrv
C:>AsynchronousHTTPServerApp.exe http://localhost:8080/ D:\inetpub\repository


SOURCE FILES
=============
main.c
handler.c
common.h
AsynchronousHTTPServerApp.vcproj
AsynchronousHTTPServerApp.sln

SEE ALSO
=========
For more information on HTTP.sys APIs, go to:
http://msdn.microsoft.com/en-us/library/aa364510(VS.85).aspx


==================================
© Microsoft Corporation
