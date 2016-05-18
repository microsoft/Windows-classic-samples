------------------------------------------------------------------------------

	Copyright (C) Microsoft Corporation. All rights reserved.

------------------------------------------------------------------------------

Sample:	MQTRANS		

Purpose: 
C++ console app that demonstrates how to send/receive coordinated transactions that include MSMQ messages and SQL 
database updates.  In this case, sums of money are transferred between the sender and the receiver via messages and 
each transaction sum is recorded in a database.

Requirements:   
		MSMQ1.0 or later
		SQL 6.5
	       	ODBC
		DS enabled computer (application fails and exits if local computer is DS disabled)

Before running the sample, you must add MSMQDemo as DSN: 
		Control Panel - Administrative tools - Data Sources (ODBC) - User DSN -
		- add - SqlServer - DataSourceNAme=MSMQDemo, Server=Local

Overview:
The mqtrans application takes a single param: either "-s" or "-r".  Where "-s" specifies a sending instance of the application 
and "-r" specifies a receiving instance.

A sending instance of the applications performs the following steps:
1. Creates a new database table "SenderAccount".
2. Locates a MSMQ queue of type MQTransTest and opens it. For simplicity, this sample assumes there's only 
   one queue of this type. Note that the sample will fail if this assumption is wrong.
3. In a loop:
          Prompts the user to enter TransferSum.
          Creates a transaction using MS DTC.
          Within the transaction:
               Updates "SenderAccount" database (subtracts TransferSum).
               Sends a message to Receiver side.
          Commits the transaction.
   The loop is terminated if the user enters a zero TransferSum.
4. Cleanup.

A receiving instance of the application does the following:
1. Creates database "ReceiverAccount".
2. Creates a MSMQ a local public queue (with the Transactional property) of type MQTransTest and opens it.
3. In a loop:
          Creates a transaction using MS DTC.
          Within the transaction:
               Receives a message from the queue (with the TransferSum).
               Updates "ReceiverAccount" database (adds TransferSum).
          Commits the transaction.
4. Cleanup.

