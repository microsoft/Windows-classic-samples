========================================================================
   SAMPLE : FaxStartPrintJob API
========================================================================

Description:
-----------------

This sample illustrates how you can use the fax API. In addition to the specified coverpage, it will send a sample 1-page document.

Usage:
---------

To send fax using FaxStartPrintJob API: PrintSDICPP.exe /s <FaxServer> /p <FaxPrinterName> /f <FaxNumber>  /n <SenderName> /c <CoverPage> 

e.g. PrintSDICPP.exe /f 1234 /n abcd /c confident.cov 

If /s paramater is not given then the default Fax Server is the local server. 
If /p is not given then the default fax printer name is "Fax".