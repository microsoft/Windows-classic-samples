Asynchronous Pipe File Replication Service Sample

======
SUMMARY
======

The Async Pipe File Replication Service builds on the Multi-user FileRep Service.
It fixes some of the inadequacies of its design as described in the ReadMe, and demonstrates the following:

  Using asynchronous RPC
  Using pipes with asynchronous RPC
  Errors handling in async scenarios
  Using async file IO
  Using the completion ports for IO completion notification

The sample consists of two executables: a client utility and a service.  The service is started by the service control manager.  A distinction should be made between 'local service' and 'remote service'.  Local service is the system service running on the same machine as the client app, while remote service is executing on a remote machine that is a source for the files being replicated.  Terms 'client service' and 'server service' can be used as well, particularly to emphasize the fact that replication can take place within one machine, in case the server is the same as the local computer.  The client utility requests file replication from the local server.  The local server contacts a server on the remote machine, receives the file, and writes it locally.  For the functionality of the system services and a detailed discussion of replication, see ReadMe.txt for the simple file replication service.

This sample addresses the inadequacies and the lack of scalability of synchronous RPC services.  When a client service in the simple data replication sample receives a request it creates a worker thread to handle the request.  The worker thread makes an RPC to the server service to start replication and then makes a series of RPCs to retrieve subsequent chunks of the file from the server until the entire file has been transferred.  A few minor optimizations are made but they do not affect this basic architecture.  A multi-user client service has a slightly more sophisticated thread management then the simple service.  It maintains a worker pool consisting of the RPC server threads borrowed from the runtime and of the additional worker threads created when RPC threads get returned to the runtime after a pre-defined time interval.  After their arrival, replication requests are queued and processed by worker threads which make RPCs to the server, first to start replication, and then to retrieve chunks of the file.  The difference between the simple and the multi-user samples is mostly on the client side and the dynamics of the server's behavior is the same in both cases: server receives an RPC asking for a buffer from a particular data source, it retrieves the buffer, and returns the buffer as an out-parameter of the RPC.  The fundamental problem with this design is that an RPC thread is blocked during the execution of the server code, in particular, during the data retrieval from a disk.  This means that the number of threads on the server has to be no smaller then the number of active replication requests being processed by it at any given time.  The high number of blocked server threads wastes resources, increases contention, decreases throughput, and makes the system scale poorly, to name a few problems.  The asynchronous design solves or softens all of these issues.

The async pipe service uses asynchronous IO to the data source (async ReadFile's) and asynchronous data sends to the client (async pipe pushes in the RPC model) with completion port notification mechanism to solve the problems of the synchronous model.  Client and server services are both asynchronous, but the discussion of the client is not relevant here.

The server maintains a pool of threads listening on a completion port P.  After receiving a data replication request R, the server checks if any threads are listening on P, and creates a thread to go listen if necessary.  It then queues an IO completion packet p1 to P to request the processing of R.  The thread returns to the RPC runtime after a fairly short path and without blocking.  A worker thread T1 will pick packet p1 from port P, initialize it, open the source file, and perform an asynchronous read on the source data.  The notification on the completion of the read will be posted to P, and T1 can return to other important work (again, without having blocked).  Some thread T2 (possibly T2==T1) will later pick p2 - an IO completion notification for the read from P.  T2 will issue an asynchronous send (push) to return the data to the client, it will then issue another read for this request to pre-fetch data for the next send, and will return to the completion port to wait for stuff to do.  When the push completes or the read completes, a notification will be send to the completion port P and the IO throttling will continue.

When no work is available, no completion packets will get queued to P and threads will begin to time out waiting for them.  After timing out, a thread will check if at least one worker is waiting on P, and if there is, it will exit.  Thus, when no work is available all but one worker threads will terminate, and a single thread will be left on the completion port.  If too much work is available and a worker thread leaves P with no one else waiting on P, it will create a new worker thread, so that P continues to have at least one waiter.  Thus, the thread pool will dynamically adjust to accommodate the load on the system.

An important idea behind the above design is that worker threads never block doing IO and all notifications on IO completion are asynchronous.  To improve performance, the server also overlaps reading the data and sending it to the client.  The server achieves this by trying to work with two buffers (B1 and B2) at a time for a given request: it will perform an asynchronous read from the file into B1, while sending a previously read buffer B2 to the client.  Thus, instead of waiting for a send to complete before reading the next block of data, the server will be retrieving it in parallel with the send.  Being one buffer "ahead" of the send ensures that a new send can be issued as soon as the previous one is completed (provided that the file IO has already completed).  In the current async pipe model, RPC can notify the user code of the completion of at most one push, so we keep only one send active while reading the next buffer to be pushed.

The async pipe service compares very favorably with the sync services and generally shows a fraction of the thread count and contention.

The dynamic behavior of the async client and server is very complex.  The section of the Platform SDK on async RPC and async pipes offers some necessary background.

The file replication is requested by the FileRep utility.  The functionality here is similar to the other samples, and the client utility calls RequestFile to ask for file replication.  Security is handled equivalently to the secure and muti-user samples.

From the security perspective, the client service and the server service process the request similarly to the secure file replication service.

The sample demonstrates a general approach to writing an async RPC application to minimize thread count and contention while achieving high throughput.

=====
FILES
=====

The directory contains the following files for building
the sample distributed application FileRepAsyncPipe:

File                    Description

common.h                Common declarations
common.cpp		Common definitions
DbgMsg.cpp              Definitions for debugging and tracing routines
DbgMsg.h                Declarations for debugging and tracing routines
FileRep.cpp             The client utility
FileRepClient.acf       Attribute configuration file for the client RPC functions
FileRepClient.idl       Interface definition for the client RPC functions
FileRepClientProc.cpp   Client RPC procedures
FileRepServer.acf       Attribute configuration file for the server RPC functions
FileRepServer.cpp       File replication server, an alternative to service
FileRepServer.idl       Interface definition for the server RPC functions
FileRepServerProc.cpp   Server RPC procedures
FileRepService.cpp      File replication service
makefile                Nmake file to build for Windows XP
Prof.cpp                Definitions for profiling routines
Prof.h                  Declarations for profiling routines
ReadMe.txt              Readme file for the file replication sample
Resources.cpp           Definitions for resource-management routines
Resources.h             Declarations for resource-management routines
Service.cpp             Common service function definitions
Service.h               Common service function declarations

=================
PLATFORM SUPORTED
=================

Windows Vista or later.

To build, type "nmake" at the command line.

The following environment variables should be already set:
  
  set CPU=i386
  set INCLUDE=%SDKROOT%\h
  set LIB=%SDKROOT%\lib
  set PATH=%SDKROOT%\system32;%SDKROOT%\bin

where %SDKROOT% is the root directory for the 32-bit Windows SDK.

Build the sample distributed application:

  nmake cleanall
  nmake

This builds the executable programs FileRepService.exe (service) and FileRep.exe (client).

To build a Debug version add option "DEBUG=1 to nmake

To build a version with profiling options modify the server executable to log to the appropriate location and option "PROFILE=1" to nmake

To build a version that ignores the RPC exceptions resulting from an overloaded server add an option "STRESS=1"


==================
RUNNING THE SAMPLE
==================

On the client and server (if the two are different), enter:

  FileRepServer

or create a service with the executable FileRepSerice.exe


On the client, enter:

  FileRep ServerName RemoteFileName LocalFileName

Note: The client and server applications can run on the same 
Microsoft Windows NT computer.

Several command-line switches are available to change settings for 
this program.  For a listing of the switches available from the client 
program, enter:

  FileRep -?
