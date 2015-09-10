WinHttp Proxy Sample
====================
This sample demonstrates how to determine the proxy for a particular URL using the core functionality for querying the proxy settings. 
Additional features may be added by the application/module basing their proxy code from this sample, including but not limited to:
     1. Per URL proxy cache.
     2. Network Change awareness.
     3. Bad Proxy Filter.

Applications running as a service who have an interactive user to impersonate should impersonate the user and then use this sample code to get proxy settings.

Applications without a user to impersonate should first use WinHttpGetDefaultProxyConfiguration and if no settings are configured try AutoDetection using WinHttpGetProxyForUrl followed by no-proxy if both fail.

The APIs demonstrated in this sample are:
     1. WinHttpGetProxyForUrl - 

Sample Language Implementations
===============================
This sample is available in the following language implementations:
     C++

Files
=====
WinhttpProxySample.cpp
     This file contains wmain function.
GetProxy.h
     This file contains the definition of class ProxyResolver.     
GetProxy.cpp
     This file contains the implementation of class ProxyResolver. It demonstrates the core functionality for querying the proxy settings.     

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the directory containing the sample for a specific language.
     2. Type "MSBuild.exe WinhttpProxySample.sln".

To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open File Explorer and navigate to the directory containing the sample for CPP language.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt.
     2. Type "WinhttpProxySample.exe <url>" at the command line.

        For example:

            C:>WinhttpProxySample.exe http://www.msn.com
            Getting proxy and bypass for http://www.msn.com

              Proxy: xxx.xx.xxx.xx:80;xxx.xx.xx.xx:80
              Bypass: NULL

            Attempting to connect to http://www.msn.com

            Setting winhttp proxy to xxx.xx.xxx.xx:80;xxx.xx.xx.xx:80

            Status: 200

Security Note
=============

 This sample is provided for educational purpose only to demonstrate how to use Windows 
 WinHttp API. It is not intended to be used without modifications in a production 
 environment and it has not been tested in a production environment. Microsoft assumes 
 no liability for incidental or consequential damages should the sample code be used for 
 purposes other than as intended.

SEE ALSO
========
For more information on Winhttp authentication, go to:
http://msdn.microsoft.com/en-us/library/aa384240.aspx