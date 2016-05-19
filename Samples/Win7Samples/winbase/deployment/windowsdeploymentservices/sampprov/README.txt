========================================================================
       Windows Deployment Services PXE Provider DLL : Sample Provider
========================================================================


The Windows Deployment Services PXE Server implementation can be 
subdivided into two pieces – a PXE Server and a PXE Provider. The PXE 
Server contains the core networking capability of the server solution. 
The PXE Server supports a plug-in interface. Plug-ins are also known as 
“PXE Providers”. The provider model allows for custom PXE solutions to be 
developed while leveraging the same core PXE Server networking code base. 
This sample provider allows you to create a DLL that may replace or run 
in conjunction with the existing PXE Provider, BINL, on a Windows 
Deployment Services server. The sample provider uses a text file as its
data store. It also contains code to add a Boot option for a BCD file to the DHCP 
reply packet sent out by the server.



This sample consists of the following files - 

SampProv.cpp
    This is the main DLL source file.  It contains implementations for
    all required resource DLL entry points along with some helper
    functions.

SampProv.h
   Required header file that defines helper data structures.

SampProv.sln
    This is the solution file for the Sample PXE Provider sample generated 
    using Visual Studio. This is the solution file which should be loaded
    into Visual Studio to generate the executable for this sample.

SampProv.vcproj
    This is a Visual Studio file which contains information about the 
    version of Visual Studio that generated the file, and information 
    about the platforms, configurations, and project features.

SampProv.def
    This file contains information about the DLL that must be
    provided to run with the Windows Deployment Services PXE Server 
    software.  It defines parameters such as the name and description 
    of the DLL.  It also exports functions from the DLL.

/////////////////////////////////////////////////////////////////////////////
Implementing a Custom PXE Provider Solution:

In order to implement a full end-to-end solution using the Windows 
Deployment Services PXE Server you will need:

1. A Windows Server 2008 R2 server, Windows Server 2008 Server, Windows Server 2003 SP2, 
   or Windows Server 2003 SP1 server with the Windows Deployment Services server role 
   installed 
2. A folder shared as ‘REMINST’ 
3. The WDS TFTP root (e.g. 
   HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\WDSServer\Providers\WDSTFTP\RootFolder 
   Type=REG_SZ on Windows Server 2008 R2 machines) set to the folder shared as ‘REMINST’ 
4. Network boot programs such as wdsnbp.com, pxeboot.com, pxeboot.n12, etc. 
   placed on the share that the client will download and boot into 
5. Bootmgr.exe, boot.sdi, a .WIM file containing a bootable image 
   (generally Windows PE), and an appropriate BCD file for network booting 
   the .WIM file placed on the network share

More information on network booting Windows PE may be found in the 
documentation included in the Windows Automated Installation Kit for 
Windows 7.


/////////////////////////////////////////////////////////////////////////////
Registering a Provider:


Registration of the sample provider requires two steps:
1. Create a registry key that represents your provider 
	a. Create a new key at 
           HKLM\System\CurrentControlSet\Services\WDSServer\Providers\WDSPXE\Providers 
	b. Create a new value under this key that specifies the name of your 
           provider DLL – 
		TYPE=REG_SZ
	 	NAME=ProviderDLL
		VALUE=<full path and .DLL name of your custom provider> 
2. Specify the order of your provider.
	a. Modify the key at 
           HKLM\System\CurrentControlSet\Services\WDSServer\Providers\WDSPXE
		TYPE=REG_MULTI_SZ
		NAME=ProvidersOrder
		VALUE=<name of key created in 1a. above>

The Windows Deployment Services PXE Server passes incoming PXE requests to 
the registered providers as ordered in this registry key. If you would like 
your provider to always have the first opportunity to answer an incoming PXE 
request, you should place it first in the list. 

/////////////////////////////////////////////////////////////////////////////
Configuration File:

The sample provider uses a .ini file as its configuration data store. This 
file tells the provider whether or not to answer clients and, if answering 
clients, which network boot program the client should receive.

The sample code must be compiled in to .dll form, e.g. sampprov.dll. The .ini
configuration file must be named the same as the sampleprov, 
e.g. sampprov.dll.sampprov.ini, and placed in the same directory as the .dll.

The .ini file has the following sections –
	[Configuration]
	DefaultBootProgram=<relative path to default NBP>
	DefaultBcdFile=<relative path to BCD file>

	[Devices]
	<MAC Address prepended with 0s>=<relative path to NBP>

	[BCDFiles]
	<MAC Address prepended with 0s>=<relative path to BCD File>


Devices in the [Devices] section must be input as a MAC address. The value must 
be 32 characters in length; therefore, the MAC address must be prepended by 
20 ‘0’s. The name and path of the NBP and BCD file are relative to the ‘REMINST’ share. 
If a device is not found in the [Devices] or [BCDFiles] section of the .ini file then the 
value specified as the ‘DefaultBootProgram’ and 'DefaultBcdFile' will be used.

Example:
[Configuration]
DefaultBootProgram=boot\x86\pxeboot.com
DefaultBcdFile=boot\my.bcd

[Devices]
00000000000000000000FFEEDDCCBBAA=boot\x86\pxeboot.n12
ABCDEF0123456789ABCDEF0123456789=boot\x86\abortpxe.com
00000000000000000000AABBCCDDEEFF=boot\x64\pxeboot.n12

[BCDFiles]
00000000000000000000FFEEDDCCBBAA=boot\x86\my.bcd
00000000000000000000AABBCCDDEEFF=boot\x64\my.bcd

/////////////////////////////////////////////////////////////////////////////
