
Description:

This sample demonstrates how a special DLL may be created to allow an 
administrator to exercise additional customized control over connection
acceptance and IP address assignment for dialin users connecting to an
NT Dial-Up Server/Router.  The server is configured via the registry 
to use the special DLL (or DLL's), and if the DLL(s) are not present
the Dial-Up Server/Router will fail to start, and an error will be logged
in the server's eventlog.

This sample DLL exports the following API's

	MprAdminAcceptNewConnection
	MprAdminConnectionHangupNotification

	MprAdminAcceptNewLink
	MprAdminLinkHangupNotification

	MprAdminGetIpAddressForUser
	MprAdminReleaseIpAddress


Platform:  Windows 2000 ** 
  ** or NT 4.0 + "Routing and Remote Access" add-on, which is available as a 
     free download from www.microsoft.com


Source code:
	admindll.c


Build instructions:
	If installing from Platform SDK:
		nmake
	else
		build -cD


Requirments: 

NT Server running Dial-Up Server/Router, configured with TCP/IP and dialin
ports.  The dialin ports can be any valid media (e.g. modem, ISDN, PPTP, 
etc), as long as a user can make a RAS connection to the server with TCP/IP.


Usage Instructions:

To configure the Dial-Up Server/Router to use a custom admin DLL, the 
registry must be edited as follows for each custom DLL:

	HKEY_LOCAL_MACHINE
		SOFTWARE
			MICROSOFT
				RAS

1. Add Key --> "AdminDll"

2. Under "AdminDll" key, add a String Value called "DllPath"

3. Set the value of DllPath = "<full path to the dll>" 
   e.g. "C:\TEST\ADMINDLL.DLL"

4. Restart the server to begin using the custom DLL(s).

When dialin users connect, RAS will call the appropriate API's exported by
the custom DLL, allowing customized control over connection and link 
acceptance, and IP address assignment.



Behavior of the Sample Custom DLL

This sample DLL keeps a small database of 256 IP addresses, and will dole
out an IP address when the request comes in. It will mark the assigned IP
address as used.  In addition this database will store the user name and
port name.  When MprAdminReleaseIpAddress is called, it will check the port
name and username. If it matches then it will free up the IP address.

MprAdminAcceptNewConnection keeps a small database of the connected users
and their statistics and parameter information.  During
MprAdminConnectionHangupNotification, it will match the passed structures
with the stored information.  If the information doesn't match then the DLL
will generate an error. MprAdminAcceptNewConnection will only accept a
connection if the username is not "Administrator" and the account has no
callback privileges.  Also, this DLL will only allow a user to connect 
with two or fewer links when making a Multilink connection.  This was an
arbitararily chosen scheme for demonstration purposes. The DLL developer 
can keep their own database and let users connect based on time, 
privliges, etc.  

When the sample DLL's exported API are called, results and information are
written to debug output.  If a kernel debugger is not attached, you can 
attach your debugger to the MPROUTER.EXE process (on NT4) or svchost.exe 
process (on Windows 2000 - process that has RemoteAccess service) 
to observe the debug output.
