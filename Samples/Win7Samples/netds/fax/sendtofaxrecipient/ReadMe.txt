========================================================================
   SAMPLE : Send To Fax API
========================================================================

Description:
-----------------

This sample demonstrates the use of Send To Fax APIs. There are two function supported:
1. CanSendToFax: Is Send to Fax supported?
2. SendToFax: It takes the document and attaches it to the Fax Compose form so it is ready to be sent.

PreCondition:
-------------------

This is supported for Windows Vista SKU.

Usage:
---------

To check if SendToFax is supported: SendToFaxRecipientCPP.exe /o cansendtofax

To send documents usign SendToFax: SendToFaxRecipientCPP.exe /o sendtofax /d <filename>

Multiple filenames can be spearated by semicolons: SendToFaxRecipientCPP.exe /o sendtofax /d <filename1>;<filename2>