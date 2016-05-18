WirelessHostednetwork Sample
============================
Demonstrates how to use the wireless Hosted Network feature available on Windows 7. 

On Windows 7 and later, the operating system installs a virtual device if a 
Hosted Network capable wireless adapter is present on the machine. 
This virtual device normally shows up in the “Network Connections Folder” as 
‘Wireless Network Connection 2’ with a Device Name of 
‘Microsoft Virtual WiFi Miniport adapter’ if the computer has a single wireless 
network adapter. This virtual device is used exclusively for performing 
software access point (SoftAP) connections.  
The lifetime of this virtual device is tied to the physical wireless adapter. 
If the physical wireless adapter is disabled, this virtual device will be removed as well.


Sample Language Implementations
===============================
This sample is available in the following language implementations:
     C++

Files
=====
HostedNetwork.sln
- The Visual Studio solution file for building the complete wireless Hosted Network sample

The source code for the HostedNetwork samples is located in the following folders:

\HostedNetwork - the main wireless Hosted Network application
\IcsMgr - a static utility library used to enable Internet Connection Sharing (ICS)
\inc - shared include files
\WlanMgr - a static utility library used to initialize settings for the wireless 
   HostedNetwork application, start and stop the Hosted Network. This library is
   used set wireless Hosted Network parameters (SSID and Key, for example). 

A readme.txt file in each folder describes the source files.

 
Prerequisites
=============
1. Windows 7 (for HostedNetwork feature)
2. A Hosted Network capable wireless adapter 
3. The HostedNetwork application must be run by a member of the Administrator's group
   if ICS is to be enabled.
4. If ICS is to be enabled when run on a domain-joined, the group policy for the domain
   must allow ICS to be enabled.  

To build the sample using the command prompt:
=============================================
1. Open the Command Prompt window and navigate to the directory.
2. Type msbuild HostedNetwork.sln (solution file)


To build the sample using Visual Studio 2005 (preferred method):
================================================
1. Open Windows Explorer and navigate to the  directory.
2. Double-click the icon for the HostedNetwork.sln (solution) file to open the file in Visual Studio.
3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
=================
1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
2. Type HostedNetwork.exe at the command line, or double-click the icon for [HostedNetwork] to launch it from Windows Explorer.


