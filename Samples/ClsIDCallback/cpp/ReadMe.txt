Set\Get\Remove BITS CLSID callback Sample
================================
This sample demonstrates how to set CLSID callback on the new IBackgroundCopyFile5 interface for a job.

Sample Language Implementations
===============================
C++

Files:
=============================================
ClsIDCallback.cpp
NotifyInterfaceImp.cpp
NotifyInterfaceImp.h
stdafx.cpp
stdafx.h
targetver.h
utils.cpp
utils.h
ReadMe.txt
ClsIDCallback.sln
ClsIDCallback.vcxproj

To build the sample using the command prompt:
=============================================
	1. Open the Command Prompt window and navigate to the ClsIDCallback directory.
	2. Type msbuild ClsIDCallback.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
	1. Open Windows Explorer and navigate to the ClsIDCallback directory.
	2. Double-click the icon for the ShellStorage.sln file to open the file in Visual Studio.
	3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
	1. Create a directory "C:\BitsSample" This is the directory where the file will be downloaded.
	2. Execute "ClsIDCallback.exe RegServer" from the elevated command line to register the COM server.
	3. To execute the sample code, execute "ClsIDCallback.exe Demo" from a non-elevated command line.
	4. This sample assumes that the application is running in a non-elevated context, and will not complete successfully if run from an elevated command window.
	5. If the sample is executed successfully then you should see a command window with output "Job Transferred" and you should see a file C:\BitsSample\SQL2008UpgradeTechnicalReferenceGuide.docx
	6. To unregister the COM server execute "ClsIDCallback.exe UnRegServer" from an elevated command line.
