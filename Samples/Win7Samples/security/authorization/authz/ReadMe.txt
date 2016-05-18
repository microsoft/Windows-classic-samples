THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1996 - 2006.  Microsoft Corporation.  All rights reserved.


This sample demonstrates the Authz API.  To run this sample you must first
create three user accounts with names, ‘Joe’, ‘Martha’ and ‘Bob’. After
building the sample run the server (AuthzSvr.exe.) on a computer running
Windows XP or greater.

To test the server you must run the client from the context of ‘Bob’, ‘Martha’
or ‘Joe’.  The easies way to do this is to create a command prompt running as
one of these user’s by using the ‘RunAs’ command as follows:

Runas /u:bob cmd.exe

After hitting ‘Enter’ you will be prompted for the corresponding password,
after entering the password, if user name and password are correct, a Command
Prompt window will appear running in the user’s context.

From the command prompt running in the user’s context launch the sample client
(client.exe.) with the following command line..

Client \\. Transfer 4000

This will connect to the server and attempt to initiate a ‘Transfer’ expense.
If this was launched from the context of Bob, Martha then this amount ($40.00)
should work.

The hard coded logic is that Bob is a VP, Martha is a Manager, and Joe is an
Employee. VPs can expense up to 100000000 cents, Managers can expense up to
100000 cents, and Employees can expense up to 10000 cents.  The sample
resource manager (AuthzSvr) uses the Authz API (AuthzComputeGroupsCallback.)
to place a sid for each user’s group (VP, Manager or Employee) in their
context. Then in AuthzAccessCheckCallback the sample RM dynamically determines
whether or not the user’s group sid applies by checking if the employee is
expensing a valid amount (if so the group applies.)