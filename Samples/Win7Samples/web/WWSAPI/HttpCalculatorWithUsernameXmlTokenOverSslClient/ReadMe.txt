HttpCalculatorWithUsernameXmlTokenOverSslClientExample

Description
===========

This example shows a HTTP client that uses service proxy to talk to a
calculator service, with XML security token over SSL mixed-mode
security.  In this setup, the transport connection is protected
(signed, encrypted) by SSL which also provides server authentication.
Client authentication is provided by a WS-Security username/password
pair that is used as an XML security token by the example.

Note that this client side example uses the WS_XML_TOKEN_MESSAGE_SECURITY_BINDING.  An equivalent way of
doing the same client side security using the WS_USERNAME_MESSAGE_SECURITY_BINDINGis illustrated by the
example HttpCalculatorWithUsernameOverSslClientExample.

A matching server side for both of these client side examples is
provided by the example HttpCalculatorWithUserNameOverSslServiceExample.


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

To build the HttpCalculatorWithUsernameXmlTokenOverSslClientExample sample
  1. Open the solution HttpCalculatorWithUsernameXmlTokenOverSslClientExample.sln in Visual Studio. 
  2. Change the active solution platform to the desired platform in the 
     Configuration Manager found on the Build menu.
  3. On the Build menu, click Build. 

Running the Sample
==================

To run the HttpCalculatorWithUsernameXmlTokenOverSslClientExample sample
  1. Run HttpCalculatorWithUsernameXmlTokenOverSslClientExample by clicking Start Without Debugging on the Debug menu.

� Microsoft Corporation. All rights reserved.


