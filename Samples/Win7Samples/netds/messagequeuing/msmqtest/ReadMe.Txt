------------------------------------------------------------------------------

	Copyright (C) Microsoft Corporation. All rights reserved.

------------------------------------------------------------------------------

Sample: MSMQTEST

Purpose: 
This console-app application demonstrates how to send and receive messages between processes using the MSMQ SDK in C. 
The application can be run in either "sender" or "receiver" mode, on either DS enabled or DS disabled computer.


Overview:
The mqtest application takes a single param: either "-s" or "-r".  Where "-s" specifies a sending instance of the application and "-r" specifies a receiving instance.

The application runs either in direct mode or in standard mode. If the application is running on a DS enabled computer , the user is asked whether he wishes to 
connect with a DS disabled computer. If so, the connection will be in direct mode. This means that a private queue is used and that no DS queries are made. 
Otherwise, the connection will be in standard mode, which means that a public queue is used. When running on a DS disabled computer, the connection can only be direct.

When ran in receiver mode, the application creates and opens a local queue of a specific type (indicated by setting the PROPID_Q_TYPE queue property: a constant guid 
is used by this app). Subsequently, it enters a loop that waits for a message to arrive on that queue, which is then just displayed. The loop is terminated when the message 
with the body "quit" arrives. Then the queue is deleted and the application exits.

When ran in standard sender mode, the application queries the MQIS to obtain all queues whose type matches that set by the receiving instance. In direct mode, the user 
is asked for the computer name of the receiver, and a handle to this queue is created, assuming it exists; If it doesn't, all messages will simply be lost. (To avoid that, a local
acknowledgement queue should be created and the messages sent should be configured to ask for acknowledgement messages. Then, if a NACK message is received, the 
user can be notified of the nonexistance of the queue.) 
Subsequently the sender enters a loop that prompts the user for a string which is sent as the body of a message to each of the matching queues or the private queue. 
This loop is terminated when the user enters the string "quit".  Finally, the queues are closed (not deleted) and the application exits.

Note that since the receiver deletes its queue, the receiving instance should be invoked before sending instances.



