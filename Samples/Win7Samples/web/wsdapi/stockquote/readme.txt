SUMMARY
======= 

The StockQuote sample demonstrates the basic use of the Web Services on
Devices API.  StockQuote implements the functionality described by
StockQuote.wsdl and StockQuoteService.wsdl.  The sample includes both a
client and service.


FILES
=====

Supplied files
--------------

ReadMe.txt                          This readme file

StockQuoteContract\CodeGen_All.config         Config file for this sample
StockQuoteContract\CodeGen_Client.config      Alternate config file (see note at end)
StockQuoteContract\CodeGen_Host.config        Alternate config file (see note at end)
StockQuoteContract\StockQuote.wsdl            StockQuote Contract
StockQuoteContract\StockQuoteService.wsdl     Service and binding for StockQuote contract
StockQuoteContract\StockQuote.xsd             Schema types for StockQuote contract

StockQuoteService\Service.cpp                 Service Implementation
StockQuoteService\Service.h                   Header for Service Implementation

StockQuoteClient\Client.cpp                   Client Implementation
StockQuoteClient\Client.h                     Header for Client Implementation


Generated files
---------------
These files are generated automatically by WsdCodeGen.exe, but are
included in the sample for reference.  You may rebuild these files by
building the StockQuoteContract project.

StockQuoteContract\StockQuote.idl            Interface file
StockQuoteContract\[StockQuote.h]            Header file built from StockQuote.idl
StockQuoteContract\StockQuoteProxy.cpp       Proxy class implementations
StockQuoteContract\StockQuoteProxy.h         Proxy class definitions
StockQuoteContract\StockQuoteStub.cpp        Stub function implementations
StockQuoteContract\StockQuoteTypes.cpp       Type definitions
StockQuoteContract\StockQuoteTypes.h         Type declarations and structure definitions


PLATFORMS SUPORTED
==================

Windows Vista
Windows Server 2008
Windows 7
Windows Server 2008 R2


RUNNING THE SERVER AND CLIENT APPLICATIONS
==========================================

To build, type "nmake" at the command line in this directory.

The client and service applications can run on the same Microsoft Windows Vista 
computer or a different one.

To run the service type:

  StockQuoteService.exe

To run the client type:

  StockQuoteClient.exe <device ID>  (where <device ID> is the ID printed
                                     by StockQuoteService.exe)


LAYOUT OF CLASSES AND FUNCTIONS
===============================

Service classes and functions
-----------------------------

CStockQuoteService      Implements the IStockQuote interface, which matches
                        the StockQuote port type.  This class acts like
                        a COM object (i.e., has AddRef, Release, and
                        QueryInterface methods) and also exposes the
                        GetLastTradePrice method, which can be accessed
                        across the wire.

StockLookup             Function for generating stock prices.  This is an
                        unsophisticated dummy function to demonstrate
                        basic service functionality.

wmain                   Main function for processing command-line arguments
                        and building a device host which advertises a
                        StockQuote service.

Client classes and functions
----------------------------

wmain                   Main function for processing command-line arguments,
                        creating a proxy for sending service messages to a
                        StockQuote service, and finally invoking the
                        GetLastTradePrice method on a service.


ALTERNATE CODEGEN CONFIG FILES
==============================

Three config files are included with this sample.
*   CodeGen_All.config      Default, includes both host and client options
*   CodeGen_Client.config   Client-only options
*   CodeGen_Host.config     Host-only options

Only the first of these (CodeGen_All.config) is used when generating code
for the StockQuote sample.  CodeGen_Client.config and CodeGen_Host.config
are provided to illustrate how to generate code for only the client and
only the host.

If you would like to use these alternate config files, follow these steps:
1)  Run WsdCodeGen.exe /generatecode [alternate config file] /gbc
2)  Rebuild the StockQuoteContract project, and then rebuild either the
    StockQuoteClient or StockQuoteService project.
