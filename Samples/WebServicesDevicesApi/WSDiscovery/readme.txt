WS-Discovery SDK Sample
====================================================================
Demonstrates how to use WSDAPI to perform WS-Discovery routines
through IWSDiscoveryProvider, IWSDiscoveryProviderNotify,
IWSDiscoveryPublisher and IWSDiscoveryPublisherNotify interfaces.
The SDK sample covers the following WS-Discovery message pattern:
Probe -->ProbeMatches --> Resolve --> ResolveMatches

Sample Language Implementations
====================================================================
C++

PLATFORMS SUPORTED
==================

Windows 7
Windows Server 2008 R2
Windows Developer Preview

To build the sample using the command prompt:
====================================================================
1. Open the Command Prompt window and navigate to the WSDiscovery
   directory.
2. Type msbuild WSDiscovery.sln.

To build the sample using Visual Studio (preferred method)
====================================================================
1. Open File Explorer and navigate to the WSDiscovery directory.
2. Double-click the icon for the WSDiscovery.sln file to open the
   file in Visual Studio.
3. In the Build menu, select Build Solution.  The application will
   be built in the default \Debug\Win32 or \Release\Win32 directory
   if it is being built for an x86 environment, or \Debug\x64 or 
   \Release\x64 directory if it is being built for an x64 environment.

To run the sample:
====================================================================
1. Navigate to the directory that contains the new executable,
   using the command prompt or File Explorer.
2. To start the WS-Discovery Client, type WSDiscoveryClient.exe /?
   for a list of commands and usages.
3. To start the WS-Discovery Target Service, type
   WSDiscoveryService.exe /? for a list of commands and usages.

Note: In order for the client and target service to run correctly,
      please ensure that your firewall settings allow WS-Discovery
      traffic and that the firewall allows multicast traffic.  The
      client and target service may be run on the same or differnt
      machines, and they can be run it multiple instances
      simultaneously to mimik the effect of having multiple target
      services or clients in the network, provided that the
      machines hosting these applications have sufficient resources
      such as large enough memory.
