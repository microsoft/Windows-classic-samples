========================================================================
   SAMPLE : Fax Account Folders Enumeration
========================================================================

Description:
-----------------

This sample demonstrates the use of Fax Account Folders APIs. Faxes in Incoming, Inbox, Outbox and SentItems folders can be enumerated.

PreCondition:
-------------------

This is supported for Windows Vista Server SKU.

Note: For C# and VB.Net Samples to run, you need to copy the managed FaxComexLib.dll to the same folder as the exe.

Usage:
---------

To enumerate Inbox: FoldersEnumCPP.exe /s <FaxServerName> /o enuminbox 

To enumerate SentItems: FoldersEnumCPP.exe /s <FaxServerName> /o enumsentitems

To enumerate Incoming: FoldersEnumCPP.exe /s <FaxServerName> /o enumincoming

To enumerate Outbox: FoldersEnumCPP.exe /s <FaxServerName> /o enumoutbox 

If /s paramater is not given then the default Fax Server is the local server. 

Note: It is not possible to Enumerate other users Folders. 