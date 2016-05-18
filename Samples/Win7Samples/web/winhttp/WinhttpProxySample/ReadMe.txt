SUMMARY
=======
WinHttp application for determining the proxy for a particular URL.
This sample demonstrates the core functionality for querying the proxy settings. 
Additional features may be added by the application/module basing their proxy code from this sample, including but not limited to:
        1) Per URL proxy cache.
        2) Network Change awareness.
        3) Bad Proxy Filter.

 Applications running as a service who have an interactive user to impersonate
 should impersonate the user and then use this sample code to get proxy settings.
 Applications without a user to impersonate should first use 
 WinHttpGetDefaultProxyConfiguration first and if no settings are configured try
 AutoDetection using WinHttpGetProxyForUrl followed by no-proxy if both fail.

Security Note 
=============

 This sample is provided for educational purpose only to demonstrate how to use Windows 
 WinHTTP API. It is not intended to be used without modifications in a production 
 environment and it has not been tested in a production environment. Microsoft assumes 
 no liability for incidental or consequential damages should the sample code be used for 
 purposes other than as intended.

USAGE
=====
This sample includes Microsoft Visual Studio .NET project files. To create "Winhttp Proxy Sample.exe",
load "Winhttp Proxy Sample.sln" and build the project.

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type msbuild "Winhttp Proxy Sample.sln".


To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
==================
C:>Winhttp Proxy Sample.exe <url>
For example:
C:>Winhttp Proxy Sample.exe http://www.msn.com
Getting proxy and bypass for http://www.msn.com

  Proxy: xxx.xx.xxx.xx:80;xxx.xx.xx.xx:80
  Bypass: NULL

Attempting to connect to http://www.msn.com

Setting winhttp proxy to xxx.xx.xxx.xx:80;xxx.xx.xx.xx:80

Status: 200

SOURCE FILES
=============
BasicProxyMain.cpp
GetProxy.h
GetProxy.cpp
Winhttp Proxy Sample.vcproj
Winhttp Proxy Sample.sln

SEE ALSO
=========
For more information on Winhttp authentication, go to:
http://msdn.microsoft.com/en-us/library/aa384240.aspx


==================================
© Microsoft Corporation
