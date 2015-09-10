EAPHost Server Method Sample
============================
This sample demonstrates how to implement an EAPHost based EAP method on the server.

Sample Language Implementations
===============================
This sample is available in the following language implementations:
C++

Files
=====
EapHostServerMethod.cpp
    This file includes the functions to demonstrate how to implement the EAP authenticator APIs for developing an EAP server method.

To build the sample using the command prompt:
=============================================
    1. Open the Command Prompt window and navigate to the directory containing the sample for a specific language.
    2. Type "msbuild EapHostServerMethodSample".

To build the sample using Visual Studio 2011 (preferred method):
================================================================
    1. Open File Explorer and navigate to the directory containing the sample for CPP language.
    2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
    3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
==================
    1. Navigate to the directory that contains EapHostServerMethodSample.dll using the command prompt.
    2. Type regsvr32 EapHostServerMethodSample.dll at the command line.
