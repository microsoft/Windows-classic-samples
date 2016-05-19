---------------------------------------------------------------------------
    Copyright (c) 1998  Microsoft Corporation
    Module Name: WSnmpUtil.exe    
    File       : ReadMe.txt
    Description: This sample demonstrates the usage of WinSNMP Manager APIs
                 through the development of a SNMP Manager Application.
                 It supports the GET, GETNEXT, and SET operations of SNMP 
                 Version 1,  and the GETBULK operation of SNMP Version 2. 
                 The extension operation WALK and SUBTREE are implemented 
                 using the GETNEXT operation.
----------------------------------------------------------------------------



---------------------
Supported Platforms:
---------------------
Windows 2000 or later.


---------------------
Building this sample:
---------------------
To build the sample application, you need Windows 2000 Beta 2
Platform SDK or later to obtain the correct headers and libs

  NMAKE



----------------------------------
Examples of using WSnmpUtil.exe:
----------------------------------


Listening for Trap
==================
Start wsnmputil to listen for traps sending to your machine by
C:\>wsnmputil trap
WSnmpUtil: listening for traps...

Stop and start your SNMP service which is configured to send trap
to the localhost as one of the trap destinations by
C:\net stop snmp
C:\net start snmp

You will then see the traps received by wsnmputil similar to the following:
C:\>wsnmputil trap
WSnmpUtil: listening for traps...
Agent : 127.0.0.1

OID :1.3.6.1.2.1.1.3.0
OID string: system.sysUpTime.0
TIMETICKS: 0

OID :1.3.6.1.6.3.1.1.4.1.0
OID string: .iso.org.dod.internet.6.3.1.1.4.1.0
OBJECT IDENTIFIER - .1.3.6.1.6.3.1.1.5.1
ColdStart

OID :1.3.6.1.6.3.1.1.4.3.0
OID string: .iso.org.dod.internet.6.3.1.1.4.3.0
OBJECT IDENTIFIER - .1.3.6.1.4.1.311.1.1.3.1.2

Enterprise specific oid from v1 trap


Get operation
=============
*Get the value of toasterManufacturer on toaster agent
C:\>wsnmputil -v1 get toaster public 1.3.6.1.4.1.12.2.1.0
OID :1.3.6.1.4.1.12.2.1.0
.iso.org.dod.internet.private.enterprises.12.2.1.0
OCTET STRING - Microsoft Corporation


GetNext operation
=================
*Get the lexicographic successor of toasterManufacturer on toaster agent
C:\>wsnmputil -v1 getnext toaster public 1.3.6.1.4.1.12.2.1.0
OID :1.3.6.1.4.1.12.2.2.0
.iso.org.dod.internet.private.enterprises.12.2.2.0
OCTET STRING - Example SNMP Extension Agent for Windows/NT (TOASTER-MIB).


Subtree operation
=================
*Get the subtree of the toaster mib on toaster agent
C:\>wsnmputil -v1 subtree toaster public 1.3.6.1.4.1.12.2
OID :1.3.6.1.4.1.12.2.1.0
.iso.org.dod.internet.private.enterprises.12.2.1.0
OCTET STRING - Microsoft Corporation


OID :1.3.6.1.4.1.12.2.2.0
.iso.org.dod.internet.private.enterprises.12.2.2.0
OCTET STRING - Example SNMP Extension Agent for Windows/NT (TOASTER-MIB).


OID :1.3.6.1.4.1.12.2.3.0
.iso.org.dod.internet.private.enterprises.12.2.3.0
INTEGER: 1


OID :1.3.6.1.4.1.12.2.4.0
.iso.org.dod.internet.private.enterprises.12.2.4.0
INTEGER: 2


OID :1.3.6.1.4.1.12.2.5.0
.iso.org.dod.internet.private.enterprises.12.2.5.0
INTEGER: 3


Set operation
=============
*Set toasterToastType on toaster agent to 2
C:\>wsnmputil -v1 get toaster public 1.3.6.1.4.1.12.2.5.0
OID :1.3.6.1.4.1.12.2.5.0
.iso.org.dod.internet.private.enterprises.12.2.5.0
INTEGER: 3

C:\>wsnmputil -v1 set toaster public 1.3.6.1.4.1.12.2.5.0 2
Succeeded in setting the OID value ..

C:\>wsnmputil -v1 get toaster public 1.3.6.1.4.1.12.2.5.0
OID :1.3.6.1.4.1.12.2.5.0
.iso.org.dod.internet.private.enterprises.12.2.5.0
INTEGER: 2


GetBulk operation
=================
*Get the lexicographic successor of sysObjectID and 2 lexicographic successors 
 of ipNetToMediaPhysAddress and ipNetToMediaType in 1 getbulk request on toaster agent:
C:\>wsnmputil -v2 getbulk toaster public 1 2 1.3.6.1.2.1.1.2.0  1.3.6.1.2.1.4.22.1.2  1.3.6.1.2.1.4.22.1.4
OID :1.3.6.1.2.1.1.3.0
system.sysUpTime.0
TIMETICKS: 8504211


OID :1.3.6.1.2.1.4.22.1.2.6.157.54.176.1
ip.ipNetToMediaTable.ipNetToMediaEntry.ipNetToMediaPhysAddress.6.157.54.176.1
OCTET STRING - <0x00><0x10><0x54><0x42><0xc0><0x88>


OID :1.3.6.1.2.1.4.22.1.4.6.157.54.176.1
ip.ipNetToMediaTable.ipNetToMediaEntry.ipNetToMediaType.6.157.54.176.1
INTEGER: 3


OID :1.3.6.1.2.1.4.22.1.2.6.157.54.178.219
ip.ipNetToMediaTable.ipNetToMediaEntry.ipNetToMediaPhysAddress.6.157.54.178.219

OCTET STRING - <0x00><0xc0><0x4f><0xad><0x89><0x5e>


OID :1.3.6.1.2.1.4.22.1.4.6.157.54.178.219
ip.ipNetToMediaTable.ipNetToMediaEntry.ipNetToMediaType.6.157.54.178.219
INTEGER: 3


Walk operation
==============
*Walk all mibs on agent toaster starting from toasterToastType
C:\>wsnmputil -v1 walk frankli0 public 1.3.6.1.4.1.12.2.5.0
OID :1.3.6.1.4.1.77.1.1.1.0
.iso.org.dod.internet.private.enterprises.lanmanager.lanmgr-2.common.comVersionMaj.0
OCTET STRING - 4


OID :1.3.6.1.4.1.77.1.1.2.0
.iso.org.dod.internet.private.enterprises.lanmanager.lanmgr-2.common.comVersionMin.0
OCTET STRING - 0


OID :1.3.6.1.4.1.77.1.1.3.0
.iso.org.dod.internet.private.enterprises.lanmanager.lanmgr-2.common.comType.0
OCTET STRING - <0x03><0x00><0x00><0x00>


OID :1.3.6.1.4.1.77.1.1.4.0
.iso.org.dod.internet.private.enterprises.lanmanager.lanmgr-2.common.comStatStart.0
INTEGER: 894297856

.
.
.

