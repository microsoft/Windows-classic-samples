SUMMARY
=======
This sample demonstrate the use of Winhttp APIs to send asynchronous requests to a server and how to cancel such requests.

Security Note 
=============

 This sample is provided for educational purpose only to demonstrate how to use Windows 
 WinHTTP API. It is not intended to be used without modifications in a production 
 environment and it has not been tested in a production environment. Microsoft assumes 
 no liability for incidental or consequential damages should the sample code be used for 
 purposes other than as intended.

USAGE
=====
This sample includes Microsoft Visual Studio .NET project files. To create "Winhttp Async Sample.exe",
load AuthDefaultCred.sln and build the project.

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type msbuild "Winhttp Async Sample.sln".


To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
==================
C:>"Winhttp Async Sample.exe" <servername>


SOURCE FILES
=============
main.cpp
Winhttp Async Sample.vcproj
Winhttp Async Sample.sln

SEE ALSO
=========
For more information on Winhttp asynchronous transactions, go to:
http://msdn.microsoft.com/en-us/library/aa383138(VS.85).aspx


==================================
© Microsoft Corporation
