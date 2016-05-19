========================================================================
   SAMPLE : Fax Sever Configuration
========================================================================

Description:
-----------------

This sample demonstrates the use of Fax Server Configuration APIs. It includes setting Personal Coverpages, enabled/disabled branding, autocreate account on connect and reassign to be set on or off.
It can be extended to set other configuration values available in FaxConfiguration Interface.

PreCondition:
-------------------

Setting of Fax Server Configuration  will work 
1. If the user has "Query Server Config" and "Manage Server Config" ACE 
2. Running in elevated mode.

This is supported for Windows Vista Server SKU.

Note: For C# and VB.Net Samples to run, you need to copy the managed FaxComexLib.dll to the same folder as the exe.

Usage:
---------

To set the "AllowPersonalCoverPages" option: ServerConfigCPP.exe /s <FaxServerName> /o PersonalCoverPage /v 1

To set the "Branding" option: ServerConfigCPP.exe /s <FaxServerName> /o Branding /v 1

To unset the "AutoCreateAccountonConnect" option: ServerConfigCPP.exe /s <FaxServerName> /o AutoCreateAccount /v 0

To unset the "IncomingFaxesArePublic" option: 	ServerConfigCPP.exe /s <FaxServerName> /o IncomingFaxesPublic /v 0

/v can take values 0 or 1. 0 is set to false and 1 is set to true.
If /s paramater is not given then the default Fax Server is the local server. 