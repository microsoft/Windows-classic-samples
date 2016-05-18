==============================================
Networking, Protocols, Asynchronous Operations
==============================================
Last Updated: Mar 30 - 2006         


SUMMARY
========
The async sample demonstrates the asynchronous use of WinInet APIs. This application can be used to:
* Download resources from a web server.
* Upload resources to a web server (POST).
* Connect to a web server using a proxy. 
* Establish a secure communication using SSL.


USAGE
======
This sample includes Microsoft Visual Studio .NET project files. To create async.exe,
load async.sln and build the project.

With Visual Studio 6.0, you need to create a blank console application and add async.c, async.h and precomp.h 
to the project.
You must also add wininet.lib to the linker's list of libraries. 

To run the sample:

1.  If you have a direct connection to the Internet, type: 
    c:> async.exe   
2.  If you have a proxy connection to the Internet, type: 
    c:> async.exe -p <proxyname>
3.  If you want to display the usage of the application, type:
    c:> async.exe -?
    
By default (without parameters), the async demo application will send a GET request
asynchronously to www.microsoft.com and it will create a temporary file to store the 
response sent by the server.

The use of the application is as follows:
async [-a {get|post}] [-h <hostname>] [-o <resourcename>] [-s] [-p <proxyname>] 
[-w <output filename>] [-r <file to post>] [-t <userTimeout>]
Flag Semantics:
-a : Specify action ("get" if omitted)
-h : Specify Hostname ("www.microsoft.com" if omitted)
-o : Specify resource name in the server ("/" if omitted)
-s : Use secure connection - https
-p : Specify Proxy
-w : Specify file to write output to (generate temp file if omitted)
-r : Specify file to post data from
-t : Specify time to wait for completing the operation in async mode. Default 2 minutes


For example, to post the content of a file (postcontent.txt) to http://www.foo.com/bar.asp 
using the demo application you should use:

	async -a post -h www.foo.com -o bar.asp -r postcontent.txt

This will cause the application to send the content of the postcontent.txt file to the bar.asp
resource in the server and create a temporary file to store the server response (if any).

The application will also show a list of the callbacks received for the handles. i.e.

	Callback Received for Handle 00CC0004   Handle cc0008 created
	Callback Received for Handle 00CC0008   Handle cc000c created
	Callback Received for Handle 00CC000C   Status: Detecting Proxy
	Callback Received for Handle 00CC000C   Status: Cookie found and will be sent with request
	Callback Received for Handle 00CC000C   Status: Resolving Name
	Callback Received for Handle 00CC000C   Status: Name Resolved
	Callback Received for Handle 00CC000C   Status: Connecting to Server
	Callback Received for Handle 00CC000C   Status: Connected to Server
	Callback Received for Handle 00CC000C   Status: Sending request
	Callback Received for Handle 00CC000C   Status: Request sent (375 Bytes)
	Callback Received for Handle 00CC000C   Status: Receiving Response
	Callback Received for Handle 00CC000C   Status: Response Received (1024 Bytes)

and so on.

BROWSER/PLATFORM COMPATIBILITY
===============================
This sample is supported in Internet Explorer 6 or later on the 
Windows platform.
 

SOURCE FILES
=============
async.c
async.h
precomp.h
async.vcproj
async.sln

SEE ALSO
=========
For more information on Asynchronous use of WinInet, go to:
http://msdn.microsoft.com/library/en-us/wininet/wininet/asynchronous_operation.asp



==================================
© Microsoft Corporation  
