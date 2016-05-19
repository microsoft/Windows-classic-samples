========================================================================
   SAMPLE : Fax Accounts
========================================================================

Description:
-----------------

This sample demonstrates the use for Fax Accounts specific APIs. It includes addition, deletion, 
enumeration of accounts.

Note: Addition, Deletion and Enumeration will need to the user to have Fax Manage Config ACE and  also has to be run in elevated mode.

This is supported for Windows Vista.

Note: For C# and VB.Net Samples to run, you need to copy the managed FaxComex.dll to the same folder as the exe.

Usage:
---------

To enumerate the available accounts on the server: "FaxAccount /s FaxServer /o enum"

To add a fax account: "FaxAccount /s FaxServer /o add /a domain\testuser"

To delete a fax account: "FaxAccount /s FaxServer /o delete /a domain\testuser"

To validate if a account is present: "FaxAccount /s FaxServer /o validate  /a domain\testuser"

