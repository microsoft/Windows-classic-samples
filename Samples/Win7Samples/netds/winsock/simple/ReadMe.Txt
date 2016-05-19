Simple IP samples. 
-----------------

simples.cpp:
----------

This is a very simple-minded TCP/UDP server. It listens on a specified port
for client connections. When a client connects, the server receives data,
echoes it back to the client and closes the connection.

Usage:
	simples -p <protocol> -e <endpoint> -i <interface>

Where,
 Protocol is one of TCP or UDP,

 Endpoint is the port number to listen on,

 Interface is the IP address to bind to (for multihomed machines this can be
   specified. Machines with just one network interface will not need this
   parameter, typically).


	Note:
	----

	There are differences in the way TCP and UDP "servers" can be written. For
	TCP, the paradigm of bind(), listen() and accept() is widely implemented. 
	For UDP , however, there are two things to consider:

    1. listen() or accept() do not work on a UDP socket. There are APIs
	that are oriented towards connection establishment, and are not applicable
	to datagram protocols. To implement a UDP server, a process only needs to
	do recvfrom() on the socket that is bound to a well-known port. Clients
	will send datagrams to this port, and the server can process these.

	2. Since there is no connection esablished, the server must treat each
	datagram separately.


simplec.cpp
---------

A simple TCP/UPD client application. It connects to a specified IP address and
port and sends a small message. It can send only one message, or loop for a
specified number of iterations, sending data to the server and receiving a
response.

Usage:
	simplec -p <protocol> -n <server> -e <endpoint> -l <iterations>

Where,
 Protocol is TCP or UDP.
 Server is the IP address/ name of the server
 Endpoint is the port number the server is listening on
 iterations specifies how many messages to send. 
 
 '-l' without any arguments will cause the client to send & receive  messages 
 until interrupted by Ctrl-C.
 This option *will not* work against simples.exe as the TCP server. This is
 because simples.exe closes the socket after one transaction. You will need
 something like the overlap sample which keeps the socket open indefinitely.




	Note:
	----
	As explained for simples.c, there is no concept of a connection in UDP
	communications. However, we can use connect() on a UDP socket. This
	establishes the remote (IPaddr, port) to used when sending a datagram.
	Thus, we can use send() instead of sendto() on this socket.

	This makes the code exactly the same for UDP and TCP sockets. However, it
	must be realized that this is still connectionless datagram traffic for
	UDP sockets, and must be treated as such.


simples_ioctl.cpp
-------

This is a TCP-only server that shows how to use select() in a Win32 console
application. The server creates a socket to listen() on, and makes it
non-blocking with ioctlsocket(), and then calls select().
When a client connects to this server, the server multiplexes between the new
connection and the original listening socket, again using select(). 
As soon as a new connection is established, the server breaks the connection
with the first client.

Usage: (same as simples)
	
This sample illustrates the point that select() is not very useful in Win32
applications, since it is difficult to keep track of every open socket and
test it for readiness when select() returns.

Applications should use WSAAsyncSelect for GUI-based event
selection, or Overlapped I/O and native Win32 wait functions to achieve the
same effect as select.
