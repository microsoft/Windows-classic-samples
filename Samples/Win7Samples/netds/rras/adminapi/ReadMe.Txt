
Description:

This sample exercises MprAdmin API's against an NT 5.0 Dial-Up Server/Router
to enumerate info on connected ports and users, to retrieve and clear port
statistics, to reset and disconnect ports, and to retreive and set users'
dialin and callback privileges.


Platform: Windows 2000 ** 
  ** or NT 4.0 + "Routing and Remote Access" add-on, which is available as a 
     free download from www.microsoft.com


Requirements: 

NT Server running Dial-Up Server/Router, configured with dialin ports.  The
dialin ports can be any valid media (e.g. modem, ISDN, PPTP, etc), as long
as a user can make a RAS connection to the server.


Source code:
	admapit.c

Build instructions:
	If you have installed Platfor SDK then execute command:
		nmake 
	else
		build -cD

Usage:

1. Run this console application via the command-line:

	ADMAPIT \\SERVERNAME

   where \\SERVERNAME is the computername of a local or remote NT Dial-Up
   Server/Router,   which is currently running and has ports configured 
   for remote users to dialin.

2. Make a RAS connection to the server, and ADMAPIT will cycle through and
   enumerate port and user info, dumping statistics, resetting the port, 
   manipulating user permissions, and finally disconnecting the RAS 
   connection.  View results in the debugger and/or in three log files 
   (*.LOG) which are created in the directory where ADMAPIT is run.


List of API's exercised in the sample application:

	MprAdminIsServiceRunning

	MprAdminServerConnect
	MprAdminServerDisconnect

	MprAdminPortEnum
	MprAdminPortGetInfo
	MprAdminPortClearStats
	MprAdminPortReset
	MprAdminPortDisconnect

	MprAdminConnectionEnum
	MprAdminConnectionGetInfo
	MprAdminConnectionClearStats

	MprAdminGetPDCServer
	MprAdminUserGetInfo
	MprAdminUserSetInfo

	MprAdminBufferFree

	MprAdminGetErrorString


