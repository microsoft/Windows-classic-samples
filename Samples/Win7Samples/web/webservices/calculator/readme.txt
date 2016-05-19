==================================
WebService - Calculator
==================================
Last Updated: OCT. 18,  2001              


SUMMARY
========
Element behaviors are a new technology in Microsoft® Internet Explorer 5.5. They 
allow Web authors to encapsulate reusable functionality inside an HTML component 
(HTC) file. A lightweight HTC--an HTC with less memory overhead that doesn't come 
with its own document--enables users to create lightweight components and to reuse them 
many times on the same page.  


DETAILS
========
The Web Service behavior is implemented as an attached behavior to a single element
within the HTC file: in this case, the BODY object. This enables you to invoke methods
from one or more different Web Services. The Web Service behavior supports a wide
variety of data types including: intrinsic SOAP data types, arrays, objects, arrays of
objects, and Extensible Markup Language (XML) data. Within the primary document, there
are two custom tags, myCalculator:calculator and myMenu:menu. The script that
implements each of the custom tags is encapsulated in its own separate HTC file. 


USAGE
======
To use the Web Service Calculator, complete the following steps.
1. Download and install Microsoft .NET Framework SDK Beta 2.
2. Download and install Internet Explorer 5.5 or greater if you are not running the
   minimum browser required.  
3. Download the files from this sample and copy to the same folder created in step 2.
4. Type http://localhost/MyWebProject/folder/calculator.hta in your Web browser and
   choose Open from the dialog window that appears.


BROWSER/PLATFORM COMPATIBILITY
===============================
The Web Service behavior is supported in Internet Explorer 5 and later and can be used
with the following products, and other products that support WSDL 1.1.
 - Microsoft .NET Framework SDK Beta 2
   - http://www.microsoft.com/net/
 - SOAP Toolkit Version 2 Beta 2
   - http://msdn.microsoft.com/code/sample.asp?url=/msdn-files/027/001/580/msdncompositedoc.xml
 - Microsoft Visual Studio .NET Beta 2
   - http://msdn.microsoft.com/vstudio/
 - Apache
   - http://xml.apache.org/soap/

Element behaviors are supported in Internet Explorer 5.5 and later on the Microsoft®
Win32 platform. Because this sample uses element behaviors and attached behaviors, use
Internet Explorer 5.5 or later to run this sample successfully.


SOURCE FILES
=============
calc.ico
calculator.asmx
calculator.hta
calculator.htc
menu.htc
webservice.htc

OTHER FILES
=============
Readme.txt

SEE ALSO
=========
For information on ASP .NET Developer's Resources, go to:
http://msdn.microsoft.com/net/aspnet/default.asp

For information on DHTML Behaviors, go to:
http://msdn.microsoft.com/workshop/author/behaviors/overview.asp

For information on the DHTML Object Model, go to:
http://msdn.microsoft.com/workshop/author/default.asp

For information on Element Behaviors, go to:
http://msdn.microsoft.com/workshop/author/behaviors/overview/elementb_ovw.asp

For the HTC reference, go to:
http://msdn.microsoft.com/workshop/components/htc/reference/htcref.asp

For information on the .NET Framework, go to:
http://msdn.microsoft.com/library/dotnet/cpguide/cpguide_start.htm

For information on Microsoft Internet Explorer, go to:
http://msdn.microsoft.com/ie/

For information on Microsoft .NET, go to:
http://www.microsoft.com/net/default.asp

For information on WebServices, go to:
http://msdn.microsoft.com/webservices/

==================================
© Microsoft Corporation 
