ReadMe.txt

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1998  Microsoft Corporation.  All Rights Reserved.

Author: Frank K. Li - Microsoft Developer Support

Files:
======
The RnrCs application uses the following files

 File           Description
 ------------------------------------------------------
 ReadMe.txt     Readme file for the RnrCs application
 RnrCs.cpp      source code file
 Makefile       Nmake file


Winsock 2 RnR API Used:
=======================
 WSAInstallServiceClass
 WSASetService
 WSAEnumNameSpaceProviders
 WSALookupServiceBegin
 WSALookupServiceNext
 WSALookupServiceEnd


Abstract:
=========
      In a traditional Winsock 1.1 client/server program, the server 
      binds itself to a well known port of a particular protocol family. 
      Then, any client can talk to the server only if it knows the server's 
      addressing infomation which includes the server's address, protocol, 
      and socket type.

      The Winsock 2 Name Resolution and Registration (RnR) provides a unified
      API set for client to search for registered services without having any
      prior knowledge of addressing information. Basically, a server wants to 
      register its existence within one or more name spaces through a friendly
      service instance name and associate itself to a service class GUID.
      Client application will be able to find the addressing 
      information of a server by the friendly name and class GUID
      in a name space.
      
      In this sample, the server program converts the user supplied Type Id 
      into a service class GUID by SVCID_NETWARE() macro. (You can also 
      generate your own service class GUID using uuidgen.exe or guidgen.exe 
      and distribute this GUID to your client program.) It then checks the 
      availability of various kind of name space providers and install this 
      service class by filling out the relevant service class information 
      with NS_NTDS (NT Directory Service name space provider available in Windows 2000)
      and/or NS_SAP (Service Advertising Protocol name space provider available 
      both in Windows NT 4 and Windows 2000) name spaces whenever they are available.
      If NS_NTDS has been installed, it binds itself to UDP protocol addresses
      and a port number dynamically chosen by the system. If NS_SAP has been
      installed, it also binds itself to a IPX protocol address and a socket
      number chosen by the system. After filling out its bound socket addresses,
      it advertises its availability through a service name supplied by a user.
      For each bound non-blocking socket addresses, the server tries to
      receive any datagram sent by any client and print out the client's socket
      addressing information if a datagram is received.
      When a user hits CTRL-C, the server program will unregister this 
      service instance, remove the service class and close all sockets.

      The client program converts the user supplied Type Id into a service
      class GUID by SVCID_NETWARE() macro, such that it can find any SAP
      predefined services (e.g. file server Type Id is 4) or the corresponding 
      advertised server program. If the user supplies a service name, it 
      will just look up the service with the same service name and Type Id. 
      If the user supplies a wild card "*" for the service name, it will try 
      to look up all registered services with the same
      Type Id. For each service program found, it prints out the socket addressing
      information of the service program and sends a message to it.  

      NOTE: 
      (1) To exploit the NS_NTDS name space (Active Directory):
          (a) client and server machines should have Windows 2000 or later installed and both 
              machines should be members of a Windows 2000 domain.
          (b) If the server program is running in the security context of Domain Admins, 
              you should have no problem in publishing the service. 
          (c) If the server program is running in the context of a Domain Users, Domain
              Admins have to modify the access control list (ACL) on the WinsockServices 
              container:
              From the Active Directory Users and Computers MMC snap-in, access the View menu 
              and select Advanced Features. Under the System container is another container 
              listed as WinsockServices. Open the WinsockServices container, 
              right-click Properties->Security, add your desired user/group with Full Control
              permission. Make sure this applies onto "this object and all child objects"
              by clicking the "Advanced..." button and "View/Edit..." your selected permission
              entry. 
      (2) To advertise in the SAP name space, server program machine should have
          NWLink IPX/SPX Compatible Transport protocol and SAP Agent service installed.
      (3) To lookup servers in the SAP name space, client program machine should have
          NWLink IPX/SPX Compatible Transport protocol installed.
      (4) When the server is running on a mutihomed machine, the SAP name space
          provider will only make the first registered ipx address available, other
          ipx addresses are ignored.



Supported Platforms:
====================
 Windows 2000 Beta 2 or later, Windows NT 4.0 SP3 or later.
 
 On NT4, only the SAP name space is available. IPX/SPX compatible transport protocol 
 is required. SAP Agent service is required on the server side machine.
 
 On Windows 2000, please refer to NOTE (1) above for details in configuration of 
 NS_NTDS name space.

 
    

Building this sample:
=====================
 Run nmake to use the supplied makefile or create a VC project for 
 RnrCs. When creating a project in VC, remember to link with the 
 Winsock 2 library ws2_32.lib.  Also, use the 
 headers and libs from the April 98 Platform SDK or later.



Usage:
======
 RnrCs
 -c (running this program as a client)
 -s (running this program as a server, default is running as a server)
 -t server_type_id (default is: 200)
 -n server_instance_name (default is: MyServer)
 -p provider_name (default is: NS_ALL)
                  (e.g. NS_NTDS, NS_SAP, NS_ALL)
  Examples:
   RnrCs -s -n MyServer ....Run as server /w server_type of 48, service
                            instance name of MyServer
   RnrCs -c -n * ..............Run as client and find all servers /w server_type_id
                               of 200 from all available name spaces
   RnrCs -c -n * -p NS_SAP ....Run as client and find all servers /w server_type_id 
                               of 200 from the SAP name space
   RnrCs -c -t 4 -n * .........Run as client and find all SAP file servers
   Hit Ctrl-C to quit the server program


Examples:
=========
-running RnrCs as a server on a Windows 2000 machine in domainA with server
 instance name of "NT5HostBhelloSrv" and type id of "200"
C:\>rnrcs -s -n NT5HostBhelloSrv
Installing ServiceClassName: NT5HostBhelloSrv Type 200
NTDS name space class installation
SAP name space class installation
HostName: DEMO5DC
IP addresses bound...
                      187.91.239.60:3352
IPX address bound...
              9D36B800.00C04FB9D78C:4170
Advertise server of instance name: NT5HostBhelloSrv ...
Wait for client talking to me, hit Ctrl-C to terminate...
received: [A message from the client: NT5HostnameC : 9D36B800.00AA00A21FF1:4636]
received: [A message from the client: NT5HostnameC : 187.91.186.182:2363]

-running RnrCs as a server on a NT4 machine with server instance name of 
 "NT4HostAhelloSrv" and type id of "200"
C:\>rnrcs -s -n NT4HostAhelloSrv
Installing ServiceClassName: NT4HostAhelloSrv Type 200
SAP name space class installation
IPX address bound...
              9D36B800.00A0C9A55DC9:5DB0
Advertise server of instance name: NT4HostAhelloSrv ...
Wait for client talking to me, hit Ctrl-C to terminate...
received: [A message from the client: NT5HostnameC : 9D36B800.00AA00A21FF1:4638]

-running RnrCs as a client on a Windows 2000 machine in domainA to find servers of type id "200"
C:\>rnrcs -c -n *
Performing Query for service (type, name) = (200, *) . . .

Name[0]:               NT5HostBhelloSrv              9D36B800.00C04FB9D78C:4170
send a message to the peer...
Name[1]:               NT5HostBhelloSrv                      187.91.239.60:3352
send a message to the peer...
Name[0]:               NT4HOSTAHELLOSRV              9D36B800.00A0C9A55DC9:5DB0
send a message to the peer...
No more matches.

-running RnrCs as a client to query SAP file servers in the network
C:\>rnrcs -c -n * -t 4  -p NS_SAP
Performing Query for service (type, name) = (4, *) . . .

Name[0]:                          ITSNW              003F9578.000000000001:0451
Name[0]:                          LCMAX              0A65AFFF.000000000001:0451
Name[0]:                       NC_NW410              3401AA5C.000000000001:0451
Name[0]:                        TSUNAMI              310FC01C.000000000001:0451
Name[0]:                      HKATZ3_NW              34F1D09E.000000000001:0451
Name[0]:               GRANDMASTER_FPNW              9D396800.00C04FB67306:0451
No more matches.
