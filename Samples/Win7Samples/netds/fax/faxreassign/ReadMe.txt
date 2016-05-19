========================================================================
   SAMPLE : Fax Reassign
========================================================================

Description:
-----------------

This sample demonstrates the use for Fax Reassign APIs. It includes reassigning a fax and list the set of unassigned faxes on a Fax Server.

PreCondition:
-------------------

Reassign will work 
1. if the user has "Manage Server Receive Folder" ACE 
2. Running in elevated mode.
3. Incoming Faxes are Public is OFF.

This is supported for Windows Vista Server SKU.

Note: For C# and VB.Net Samples to run, you need to copy the managed FaxComexLib.dll to the same folder as the exe.

Usage:
---------

To get a list of Reassignable faxes: FaxReassignCPP.exe /s faxservername /o list

To reassign a fax: FaxReassignCPP.exe /o reassign /s faxservername /i MessageID /r domainname\user1;domainname\user2

Multiple recipients are separated by semicolon. MessageId is the Id of the message that is printed when  "FaxReassignCPP.exe /s faxservername /o list" is run.

If /s paramater is not given then the default Fax Server is the local server. 