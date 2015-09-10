MPR API sample
==============
The sample demonstrates 
1. How to configure custom IPSec policies on a demand dial interface.
2. How to remove custom IPSec configuration from a demand dial interface.
3. How to configure custom IPSec policies (to be applied on all the VPN connections) on a remote access (RRAS) server.
4. How to remove custom IPSec configuration from a remote access (RRAS) server.
5. How to enumerate the VPN connections on a remote access (RRAS) server. In 
order to see the enumerated VPN connections using this sample, you have to 
have few VPN clients connected to the RRAS server. Otherwise this sample would 
always show "No VPN connections are available.

The APIs demonstrated in this sample are:
1. MprAdminServerConnect - To establish a connection to a RRAS server for the purpose of administering the RemoteAccess service.
2. MprAdminServerDisconnect - To disconnect the connection made using MprAdminServerConnect.
3. MprAdminInterfaceCreate - To create a demand dial interface on a specified RRAS server.
4. MprAdminInterfaceGetInfo - To retrieve information for a specified interface on a specified server.
5. MprAdminInterfaceGetHandle - To retrieve a handle to a specified interface for administering that interface.
6. MprAdminInterfaceSetCustomInfoEx - To set tunnel specific custom configuration for a specified demad dial interface on a specified server.
7. MprAdminInterfaceGetCustomInfoEx - To retrieve tunnel specific custom configuration for a specified demad dial interface on a specified server.
8. MprAdminBufferFree - To release the memory buffers returned by the MprAdmin APIs.
9. MprAdminServerGetInfoEx - To retrieve the tunnel specific configuration about the specified RRAS server.
10. MprAdminServerSetInfoEx - To set the tunnel specific configuration on a specified RRAS server.
11. MprAdminConnectionEnum - To enumerate all active VPN connections on a specified RRAS server.
12. MprConfigServerConnect - To connect to a RRAS server to be configured.
13. MprConfigServerDisconnect - To disconnect a connection made using MprConfigServerConnect.
14. MprConfigInterfaceCreate - To create a router interface in the specified router configuration.
15. MprConfigInterfaceGetHandle - To retrieve a handle to the specified interface's configuration in the specified router configuration
16. MprConfigInterfaceSetCustomInfoEx - To set the custom IKEv2 policy configuration for a specified interface in the specified router configuration
17. MprConfigInterfaceGetCustomInfoEx - To retrieve the custom IKEv2 policy configuration for a specified interface in the specified router configuration.
18. MprConfigBufferFree - To free the buffers allocated by calls to the MprConfig APIs.
19. MprConfigServerGetInfoEx - To retrieve custom IKEv2 policy configuration from a specified server.
20. MprConfigServerSetInfoEx - To set custom IKEv2 policy configuration on a specified server.
   
Sample Language Implementations
===============================
 	This sample is available in the following language implementations:
C++
 
Files:
=============================================
main.cpp 
	This file includes the functions to demonstrate how to use the MPR APIs 
	to set and retrieve the configurations on a RRAS server.

InterfaceConfiguration.cpp 
	This file includes the functions to demonstrate how to use the MPR APIs to set 
	and retrieve the custom IPSec configuration on a demand dial interface.

ServerConfiguration.cpp 
	This file includes the functions to demonstrate how to use the MPR APIs 
	to set and retrieve the custom IPSec configuration on a RRAS server.

VpnConnection.cpp 
	This file includes the functions to demonstrate how to use the MPR APIs 
	to enumerate the active VPN connections on a RRAS server.

Utils.cpp
	This file contains the utility functions required for the MprAPiSample SDK. 

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the directory containing the sample for a specific language.
     2. Type "msbuild MprApiSample".


To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open File Explorer and navigate to the directory containing the sample for CPP language.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or File Explorer.
     2. Type MprApiSample at the command line, or double-click the icon for MprApiSample to launch it from File Explorer.
