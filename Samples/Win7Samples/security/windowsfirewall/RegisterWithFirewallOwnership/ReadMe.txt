Windows Firewall Samples (Register for Firewall Category Ownership)
===============================
     demonstrates Windows Firewall Categories APIs.


OVerall Guidance for Categories API usage
===============================

     1. Call the categories API code from a service dependent on the "Windows Firewall"
     2. Do not stop the Windows Firewall service (mpssvc) or turn of the Windows Firewall.
     3. Provide a suitable displayName at registration time


Sample Language Implementations
===============================
     This sample is available in the following language implementations:
     C++


To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the directory.
     2. Type msbuild RegisterWithFirewallOwnership.


To build the sample using Visual Studio:
=======================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     The application will be built in the default Debug directory.

Additional Build Steps:
=======================
     1. nmake


To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type RegisterWithFirewallOwnership.exe at the command line, or double-click the icon for RegisterWithFirewallOwnership.exe to launch it from Windows Explorer.


Additional Run Steps:
=====================
     1. run RegisterWithFirewallOwnership.exe
