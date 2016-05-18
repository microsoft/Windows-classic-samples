---------------------------------------------------------------------------------------------------
	Copyright (C) Microsoft Corporation. All rights reserved.
---------------------------------------------------------------------------------------------------
LargeMessageQueue Sample and Test Application


Summary 
========
LargeMessageQueue is an API that can be used to transfer large messages (greater than 4 MB) over MSMQ. It is written in C# and works as a wrapper over the System.Messaging API. 



Notes
======
• The LargeMessageQueue API can only be used with transactional queues.
• Applications must exclusively use the LargeMessageQueue API to read and send from any queue that is used for large messages (greater than 4MB). You cannot combine the use of the LargeMessageQueue Send and Receive methods and the System.Messaging.MessageQueue Send and Receive methods on the same queue (nor can you use the native or COM APIs to send or receive from the queue).
• On the receiver side, the LargeMessageQueue API needs a transactional queue named statusq to be present for correct operation.



Overview of the Sample
=======================
The LargeMessageQueue API is packaged as an independent assembly that can be added as a reference to an application project. The namespace for the API is Microsoft.Samples.MessageQueuing.LargeMessageQueue. The LargeMessageQueue class provided in the sample can be used in place of the System.Messaging.MessageQueue class for sending large messages. The receiving side of the API provides functionality corresponding to the non-cursor based Receive and ReceiveByCorrelationId methods of System.Messaging.MessageQueue only.

A test application is also included that demonstrates how to use the LargeMessageQueue API for sending and receiving user-specified files to and from a queue on the same computer. The user can send by using the normal MSMQ send or by using the LargeMessageQueue sample's Send. On successful send, the queue column and the progress bar turn green. Otherwise, the column is empty and the progress bar becomes red. The user can receive by using Lossy Receive, which simulates holes (explained in "Reassembly" in the "Design" section below) in the large message sequence, or do a complete receive, which reassembles the file from the queue.



Building the Sample
===================

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the .\CS directory.
     2. Type msbuild [Solution Filename].

To build the sample using Visual Studio 2008 (preferred method):
================================================================
     1. Open Windows Explorer and navigate to the .\CS directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. On the Build menu, click Build Solution. The application will be built in the default \Debug or \Release directory.



Running the Test Application
============================

     1. Navigate to the directory that contains the "TestLargeMessageQueue" executable, using the command prompt or Windows Explorer.
     2. The test application works with .bmp files. Copy the default.bmp (40 KB) file to the directory that contains the executable. If the file is larger than 4 MB, the normal Send operation will throw an exception.
     3. Type TestLargeMessageQueue at the command line, or double-click the icon for TestLargeMessageQueue to start it from Windows Explorer.



Design
=======
The implementation of the LargeMessageQueue API can be divided into the following pieces:

     1. Fragmentation
The large message/stream passed to the API is fragmented using the fragment size provided by the application and placed in the queue using the transaction provided by the application. However, if the application has not supplied a transaction (via a MessageQueueTransaction object) or has passed MessageQueueTransactionType (None or Single), an internal transaction (MessageQueueTransaction) is created and all the fragments are sent using this internal transaction. A header message (identified by a unique value) is sent before sending the data fragments, indicating the start of the large message sequence. A trailer message (identified by another unique value) is used to indicate the end of the large message sequence. The default values are available in the Parameters.cs file and must be less than zero. This value is put in the AppSpecific property of the control (header/trailer) messages. The fragment numbers of the data fragments are put in their AppSpecific property. The application-provided AppSpecific value is stored in the body of the control messages, so that it can be restored at the receiving side.

     2. Message Chaining
The message ID of the header message is used as the “large message sequence ID” and can be used to identify all the fragments pertaining to a particular large message. The large message sequence ID is put in the CorrelationId field of all the message fragments and the trailer message before they are sent to the queue. 

     3. Formatting
The application-provided formatting is applied to the entire large message and fragmentation is applied after that. The final formatting used by the API before sending is BinaryMessageFormatter.

     4. Reassembly
The guarantee of transactional messaging is EOIO (Exactly Once In-Order). The guarantee of the dead letter queue is to have messages that are in doubt or undelivered. Therefore messages that reach the receiving queue in a transaction may still have missing messages in them if the queue manager did not receive internal acknowledgment for some of the messages. This implies that the fragments in the large message sequence can be lost during transfer, and therefore holes in the sequence should be identified during reassembly. If holes are found then the fragments are not reassembled and a LargeMessageQueueException is thrown with the large message sequence ID that contains the missing fragment(s). 

     5. Status Queue
A message in the status queue is used to identify the sub-queue and the status of the sub-queue (complete or to-be-deleted). If the status is complete, the fragments in the sub-queue are reassembled and given to the application; else all the related fragments are rejected from the queue and a LargeMessageQueueException is thrown. If the status queue contains no messages, the receiver starts processing the messages in the receiving queue and identifies if the large message sequence is complete or to-be-deleted.
