RasAgileVPN Sample
==================
Demonstrates how to use the RasUpdateConnection and RasGetProjectionInfoEx APIs to get the
IKEv2 projection information and to perform a MOBIKE switch. First it makes a VPN connection
using RasDial on the interface specified by argument "interface_index". Once the connection is
setup it tries to perform a MOBIKE switch to the "new_interface_index" and if the switch is 
successful the local tunnel end point will be changed as seen on the console in program output.


Sample Language Implementations
===============================
C

Supported OS: 
===============================
  Windows 7, Windows Server 2008 R2


Files:
===============================
RasAgileVPN.sln
RasAgileVPN.vcproj
RasAgileVPN.cpp


To build the sample using the command prompt:
=============================================
  1. Open the Command Prompt window and navigate to the RasAgileVPN directory.
  2. Type msbuild RasAgileVPN.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
  1. Open Windows Explorer and navigate to the RasAgileVPN directory.
  2. Double-click the icon for the RasAgileVPN.sln file to open the file in Visual Studio.
  3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
  1. Navigate to the directory that contains the new executable using the command prompt.
  2. To run the application type:

     RasAgileVPN -e entry_name -p [destination_ipaddress] -u [username] -z [password]
	        -d [domain] -i [interface_index] -n [new_interface_index]

  Note: The "entry_name" is the required arguments and the rest are optional. To perform the MOBIKE 
        switch using the RasUpdateConnection API the client should support Mobile IKE (MOBIKE) and 
        the "interface_index" and "new_interface_index" arguments need to be specified and have to 
        be different. This requires 2 interfaces on the client and the VPN server should be reachable 
        from both of these interfaces. To see the available interface index type 'route print' to see
        the list of interface index under Interface List.


