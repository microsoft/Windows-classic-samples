========================================================================
   SAMPLE : Fax Send Documents
========================================================================

Description:
-----------------

This sample demonstrates the use of Fax Send Document APIs. Multiple documents can also be sent using this sample.

Note: FaxDocument->put_Body cannot be used with ConnectSubmit2 and Submit2 
         FaxDocument->put_Bodies cannot be used with ConnectedSubmit and Submit
         Both FaxDocument->put_Body and FaxDocument->put_Bodies cannot be used together.
         FaxDocument->put_Body should be used with ConnectedSubmit and Submit
         FaxDocument->put_Bodies should be used with ConnectedSubmit2 and Submit2

PreCondition:
-------------------

Note: For C# and VB.Net Samples to run, you need to copy the managed FaxComexLib.dll to the same folder as the exe.

Usage:
---------

To send a fax docuement: SendFaxCPP.exe /s <FaxServerName> /d C:\fax\fax1.txt;C:\fax\fax2.txt /n 1234

If /s paramater is not given then the default Fax Server is the local server. 
Multiple Documents can be sent by passing them in a semicolon separated string. 
