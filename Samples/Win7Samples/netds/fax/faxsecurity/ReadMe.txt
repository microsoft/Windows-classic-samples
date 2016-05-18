========================================================================
   SAMPLE : Fax Security 
========================================================================

Description:
-----------------

This sample demonstrates the use of Fax Security APIs. It include allowing or denying a secuirty ACE to a fax account.

PreCondition:
-------------------

Setting of Fax Security  will work 
1. If the user has "Query Server Config" and "Manage Server Config" ACE 
2. Running in elevated mode.

Usage:
---------

To print the list of ACEs for the various accounts: FaxSecurityCPP.exe /s <FaxServerName> /o print

To add a deny ACE for an access mask: FaxSecurityCPP.exe /s <FaxServerName> /o add /a <AccountName> /d 1 /m <AccessMask>

E.g. FaxSecurityCPP.exe /s FaxServer /o add /a testmachine\administrator /d 1 /m submit_low

If the ACE is already present then it will print the message informing that a duplicate ACE is already present for the account.

To add a allow ACE for an access mask: FaxSecurityCPP.exe /s <FaxServerName> /o add /a <AccountName> /d 0 /m <AccessMask>

To delete a deny ACE for an access mask: FaxSecurityCPP.exe /s <FaxServerName> /o delete /a <AccountName> /d 1 /m <AccessMask>

If the ACE to be deleted is not found then it will print the message indicating that the ACE is not found.

If /s paramater is not given then the default Fax Server is the local server. 

AccessMask can be any of the following values:
submit_low
submit_normal
submit_high
query_jobs
manage_jobs
query_config
manage_config
query_archives
manage_archives
manage_receive_folder