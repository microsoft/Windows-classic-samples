SUMMARY
=======
This sample demonstrate the use of Winhttp APIs to access a server with Negotiate authentication.

It builds a sinple Winhttp app that does the following:
    - Negotiate server auth
    - Synchronous POST
    - SspiPromptForCredentials

HTTP server setup:
    - Negotiate auth
    - A URL that accepts POSTs
    - copy the provided countbytes.aspx page on your server root
Example:
To setup an IIS 7 web server
============================
1. Install IIS (includes Windows Authentication) 
2. Start - Run - inetmgr 
3. Default Web Site - Authentication 
4. Right-click "Windows Authentication", Enable 
5. Copy countbytes.aspx into the site root (eg. \inetpub\wwwroot)


Security Note 
=============

 This sample is provided for educational purpose only to demonstrate how to use Windows 
 WinHTTP API. It is not intended to be used without modifications in a production 
 environment and it has not been tested in a production environment. Microsoft assumes 
 no liability for incidental or consequential damages should the sample code be used for 
 purposes other than as intended.

USAGE
=====
This sample includes Microsoft Visual Studio .NET project files. To create "Winhttp SspiPfc Sample.exe",
load "Winhttp SspiPfc Sample.sln" and build the project.

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type msbuild "Winhttp SspiPfc Sample.sln".


To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
==================
C:>"Winhttp SspiPfc Sample.exe" <servername>


SOURCE FILES
=============
main.cpp
Winhttp SspiPfc Sample.vcproj
Winhttp SspiPfc Sample.sln

SEE ALSO
=========
For more information on Winhttp authentication, go to:
http://msdn.microsoft.com/en-us/library/aa383144.aspx


==================================
© Microsoft Corporation
