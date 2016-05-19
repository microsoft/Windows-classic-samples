HttpPurchaseOrderWithKerberosOverSslClientExample

Description
===========

This example shows a HTTP client that uses service proxy to talk to a PurchaseOrder service,
with kerberos over SSL mixed-mode security.  In this setup, the
transport connection is protected (signed, encrypted) by SSL which
also provides server authentication.  Client authentication is
provided by a kerberos APREQ ticket in a WS-Security header in the
message.


Security Note 
=============


This sample is provided for educational purpose only to demonstrate how to use 
Windows Web Services API. It is not intended to be used without modifications 
in a production environment and it has not been tested in a production 
environment. Microsoft assumes no liability for incidental or consequential 
damages should the sample code be used for purposes other than as intended.

Prerequisites
=============

In order to run this sample on Windows XP, Windows Vista, Windows Server 2003
and Windows Server 2008, you may need to install a Windows Update that contains
the runtime DLL for Windows Web Services API. Please consult with the 
documentation on MSDN for more information.

Building the Sample
===================

To build the HttpPurchaseOrderWithKerberosOverSslClientExample sample
  1. Open the solution HttpPurchaseOrderWithKerberosOverSslClientExample.sln in Visual Studio. 
  2. Change the active solution platform to the desired platform in the 
     Configuration Manager found on the Build menu.
  3. On the Build menu, click Build. 

Running the Sample
==================

To run the HttpPurchaseOrderWithKerberosOverSslClientExample sample
  1. Run HttpPurchaseOrderWithKerberosOverSslClientExample by clicking Start Without Debugging on the Debug menu.

� Microsoft Corporation. All rights reserved.


