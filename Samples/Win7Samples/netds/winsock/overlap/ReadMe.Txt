overlap.c:
---------
This TCP sample demonstrates the use of AcceptEx() and overlapped I/O to 
multiplex different client connections in a single-threaded Win32 application.

AcceptEx is a Microsoft-specific extension to Windows Sockets.  It is not
part of the Windows Sockets specification.  If you use this function, you
will limit your application to only run over implementations of Windows
Sockets that support AcceptEx -- some implementations do not.

The server uses AcceptEx() to handle a number of asynchronous connection
requests effectively. When a connection is received, it is added to an array o
handles being monitored in a wait function. 
The server multiplexes between the listening socket and the various client
connections. 

When a connection is first established, the server queues an overlapped
WSARecv() operation on the socket. On subsequent notification that this
operation has completed, the server queues a WSASend(). Thus, the server does
alternate recvs and sends on the socket, until the client closes the
connection.

The simplec.exe client under the 'simple' directory can be run against this 
overlapped server.

usage:
	overlap -e <endpoint>  -i <interface>

	where,
	endpoint is the port to listen on.
	interface is the IP address to bind to for multi-homed hosts.
