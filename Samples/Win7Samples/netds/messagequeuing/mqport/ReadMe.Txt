------------------------------------------------------------------------------

	Copyright (C) Microsoft Corporation. All rights reserved.

------------------------------------------------------------------------------

Sample: mqport

Purpose: 
This console app sample demonstrates how to use NT completion ports
in MSMQ in order to asynchronously receive messages in an
efficient manner. This mechanism is scalable in the number of queues
and messages by adding more processors/threads. Likewise,
generic completion port handlers can be provided to handle other NT
resources as well as queues.
 
Both the MSMQ COM components and the MQ API are used in this program.
Note that the COM components are used for queue creation, open and message
send.  Whereas the MQ API is used to implement completion port-based
asynchronous receive.


Requirements:
VC5 is required.
MSMQ 2.0 or later must be installed -- specifically mqoa.dll must be registered and
  on the path.

Project settings:
   - the include path must include mq.h's location: 
         e.g. ...\msmq\sdk\include
   - the link library path must include mqrt.lib's location:
         e.g ...\msmq\sdk\lib

Overview:
The following steps comprise this sample:
   - DS connection is detected and a public queue will be used on a DS enabled 
     computer. Otherwise a private queue will be used.
   - A global MSMQQueueInfo object is used to reference the sample's
      single queue.
   - Initialize OLE
   - Create a new completion port 
   - Create a bunch of threads with a generic CompletionPortThread start 
      routine parameterized with the completion port handle from the previous step.
   - Open the queue and associate its handle with the completion port:
       - note the queue is deleted and recreated if already exists
          otherwise a new queue is created.
       - in case the local computer is working in workgroup mode, a private
          queue is created. A public queue is created otherwise.
   - Enable a bunch of asynchronous message receive requests on the queue.
      Since the queue is associated with the completion port, each of these
      requests will result in the CompletionPortThread handler being notified
      asynchronously by NT when the async receive message "completes".
     - note that the NT scheduler will select the "best" available completion
        port thread that is synchronously waiting for a completion notification.
   - Finally, to test the completion port handlers, a bunch of messages is sent
       to the queue and the program hibernates.
   - To exit, just kill the console app window.


For More Information:
See the MSDN help for completion ports, especially: "Writing Scalable Applications for Windows NT"
