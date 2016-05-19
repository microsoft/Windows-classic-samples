PURPOSE
=======

The tssysinfo sample demonstrates how to use Virtual Channels with Windows 2000
and Terminal Services.

Virtual Channels allow an application within a Terminal Services session to
communicate with the client machine.

The sample consists of two parts, a client DLL and a server application.
The client DLL is installed and registered on the client machine; the
server application is registered and run in the Terminal Services session.
The server application does not run in the console session.

In this sample the server application contacts the client and obtains system
information on the client system. The server application displays this
information and then obtains memory information every 2 seconds. If the
session is disconnected, the server application will sleep until the session
is reconnected.

The client DLL initializes when the client connects to the Terminal Services
machine. The DLL spawns a worker thread to send data to the server
application. The server app sends a control code indicating what kind of
information the client DLL should send back. The client DLL processes
the control code and sends back the appropriate data. The writes are
synchronous so the client DLL waits for the write to complete before
moving on.

Note:
The server must be running Windows 2000 with Terminal Services installed.
Windows NT 4, Terminal Server Edition does NOT support Virtual Channels.
The client DLL can be installed on any machine running 32 bit Windows
with a Windows 2000 compatible Terminal Services Client.

HOW TO RUN THE SAMPLE
=====================

- Modify the path in client\client.reg to point to the client DLL
  (sysinf_c.dll).
- Run client\client.reg on the client machine. (RegEdit /s Client\Client.Reg)
- Run server\server.reg on the server machine. (RegEdit /s Server\Server.Reg)

- On the client machine, start a Terminal Services Client and connect to the
  server.
- In the remote session, start a command prompt and run the server executable
  (sysinf_s.exe)

APIS USED IN THE SAMPLE
=======================

sysinfo_s.c

     ProcessIdToSessionId
     WTSVirtualChannelOpen
     WTSVirtualChannelWrite
     WTSVirtualChannelRead
     WTSVirtualChannelClose

sysinfo_c.c

     VirtualChannelWrite
     VirtualChannelOpen
     VirtualChannelInit


WHAT THE SAMPLE DOES NOT DEMONSTRATE
====================================

The sample does not demonstrate how to process chunks of data.

TIPS
====

Make sure to modify client\client.reg to have the correct path to the
client DLL.
