------------------------------------------------------------------------------

	Copyright (C) Microsoft Corporation. All rights reserved.

------------------------------------------------------------------------------

=============
MSMQ API Test
=============

MQAPITST.EXE is an MFC MSMQ application that can be used to test
and experiment MSMQ C api.

Requirements:

MSMQ 1.0 or Msmq 2.0 DS-enabled must be installed .
This sample is not designed to execute on DS-disabled computer.


Using MQAPITST.EXE you can test the following API calls:

 - MQCreateQueue
 - MQDeleteQueue
 - MQOpenQueue
 - MQCloseQueue
 - MQSendMessage
 - MQReceiveMessage
 - MQLocate (MQLocate calls the three locate APIs: MQLocateBegin,
   MQLocateNext, and MQLocateEnd)

Note
Before you can open a queue using Mqapitst.exe, you must use MQLocate
to located the queue. Mqapitst.exe locates queues based on their
labels. Also, if you are using one instance of Mqapitst.exe, you cannot send
messages to and receive messages from the same queue without first
closing the queue handle. For example, if you open the queue for send
and send five messages, you cannot read the messages without first
closing your handle to the queue, and then opening the queue for read.

The main source file for the program, where the menus are invoked and
the MSMQ APIs are called, is Mainfrm.cpp.
When building Mqapitst on Windows 95, only ANSI configurations should be used.

For more information, see the MSMQ SDK help file.

The following procedure shows you how to use MSMQ API Test to
become familiar with and experiment with the MSMQ APIs.

To see MSMQ functionality using MSMQ API Test:

1. Run two instances of MSMQ API Test (Mqapitst.exe), either on one
   computer, or two different computers.

2. Create a queue in one application, entering the Pathname (for
   example, "YourMachine\MyQueue") and the Label (for example,
   "MQ API Test") of the queue.

3. Locate the queue in the other instance of MSMQ API Test.

   You can do this by using the MQLocate API to search for the
   queue's label.

4. In one instance of MSMQ API Test open the queue for Send.

5. In the other MSMQ API Test open the queue for Receive.

6. Send and Receive messages between the applications.

   Experiment with the various send options, such as message
   priority, message time-to-reach-queue, time-to-be-received,
   acknowledgements, and delivery modes.

7. Close and then delete the queues.

