ptpcmxdw.exe is a simple application that helps demonstrate how to
1. Use PrintTicket/PrintCapabilities Win32 APIs to 
     a. Open a PrintTicket Provider
     b. Convert DEVMODE to PrintTicket 
     c. Merge PrintTickets.
2. Send a PrintTicket to a print driver that supports XPS PrintTicket passthrough escapes.
3. Send a .jpg image and FixedPage markup to a print driver that supports XPS resource and markup passthrough escapes.


Prerequisites
1. Microsoft XPS Document Writer print driver must be installed. This printer driver is PrintTicket aware and supports MXDW passthrough escapes.
2. Microsoft PrintTicket and PrintCapabilities support.  This requires that the prntvpt.dll is installed and registered on the system.
3. Microsoft MSXML 6.0 must be installed and registered on the system.
Note: All the above are installed by default on most versions Windows Vista.  If these components are not installed, please contact your system manufacturer.  On previous version of Windows (XP SP2 and later), these components may be installed individually through the Microsoft XPS Essentials Pack and MSXML 6.0 downloadable installers or by installing the .NET Framework 3.0.
