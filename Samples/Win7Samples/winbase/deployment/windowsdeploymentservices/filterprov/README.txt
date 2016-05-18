========================================================================
       Windows Deployment Services PXE Provider DLL : Filter Provider
========================================================================


The Windows Deployment Services PXE Server implementation can be 
subdivided into two pieces – a PXE Server and a PXE Provider. The PXE 
Server contains the core networking capability of the server solution. 
The PXE Server supports a plug-in interface. Plug-ins are also known as 
“PXE Providers”. The provider model allows for custom PXE solutions to 
be developed while leveraging the same core PXE Server networking code 
base. This sample provider allows you to create a DLL that may replace 
or run in conjunction with the existing PXE Provider, BINL, on a Windows 
Deployment Services server. The sample provider uses a text file as its 
data store.

This sample consists of a filter provider that illustrates how to create
an additional PXE Provider that sits before BINL (or any other PXE 
Provider) in the ordered provider list. This filter provider acts as a 
“gate” before the next provider in the list, allowing the next provider 
to service selected clients and filtering out others.

This sample consists of the following files - 

FilterProv.cpp
    This is the main DLL source file.  It contains implementations for
    all required resource DLL entry points along with some helper
    functions.

FilterProv.h
    Required header file that defines helper data structures.

FilterProv.sln
    This is the solution file which should be loaded into Visual Studio 
    to generate the executable for this sample.

FilterProv.vcproj
    This is a Visual Studio file which contains information about the 
    version of Visual Studio that generated the file, and information 
    about the platforms, configurations, and project features.

FilterProv.def
    This file contains information about the DLL that must be
    provided to run with the Windows Deployment Services PXE Server 
    software. It defines parameters such as the name and description of 
    the DLL.  It also exports functions from the DLL.

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
	b. Create a new value under this key that specifies the name of 
           your provider DLL – 
		TYPE=REG_SZ
	 	NAME=ProviderDLL
		VALUE=<full path and .DLL name of your custom provider> 
2. Specify the order of your provider.
	a. Modify the key at 
           HKLM\System\CurrentControlSet\Services\WDSServer\Providers\WDSPXE
		TYPE=REG_MULTI_SZ
		NAME=ProvidersOrder
		VALUE=<name of key created in 1a. above>

The Windows Deployment Services PXE Server passes incoming PXE requests 
to the registered providers as ordered in this registry key. If you 
would like your provider to always have the first opportunity to answer 
an incoming PXE request, you should place it first in the list. 

/////////////////////////////////////////////////////////////////////////////
Configuration File:

The sample code must be compiled into .dll form, e.g. filterprov.dll. The 
filter provider uses a configuration file that must be named 
filterprov.dll.filter.ini, and placed in the same directory as the .dll. 

The .ini file has two sections – [Configuration] and [Devices]. The 
[Configuration] section has one key, ‘Policy’, which defines whether 
unknown devices will continue to the next provider in the ordered list 
or not. The ‘Policy’ key has two values –

0 = Deny. Means that all unknown devices (those not existing in the 
          [Devices] section) will be denied PXE service by the sample 
          provider. The net effect is that the PXE request will not be 
          answered and the request will not be forwarded to the next PXE 
          Provider in the ordered list.

1 = Allow. Means that all unknown devices (those not existing in the 
           [Devices] section) will be allowed PXE service by the sample 
           provider. The net effect is that the PXE request will be 
           forwarded on to the next PXE Provider in the ordered list.


The [Devices] section lists all “known” devices. Devices must be listed 
by MAC address and the value must be 32 characters in length (MAC address 
prepended with 20 ‘0’s). For each device one can specify the desired 
action –Deny or Allow. Deny means that the sample provider will deny the 
PXE request and will not forward the request to the next PXE Provider in 
the ordered list. Allow means that the sample provider will forward on the 
PXE request to the next PXE Provider in the ordered list. It is then up to 
that next provider to service the incoming PXE request.


	[Configuration]
	Policy = [0 – Deny | 1 – Allow]

	[Devices]
	00000000000000000000FFEEDDCCBBAA=[0 – Deny | 1 – Allow]


The simple walkthrough of using the sample filter provider looks as follows:

1. Compile the sample code into a .dll
2. Create a .ini file per above
3. Register the sample provider as first in the ordered provider list
4. PXE boot a sample client. The following logic is used
		a. PXE Server hands the request off to the first provider in 
                   the ordered list. This should be the sample provider per 
                   #3 above.
		b. The sample provider looks for the booting device in the 
                   [Devices] section of the file. If it	finds it, whatever 
                   policy is specified will be followed.
		c. The sample provider looks for the ‘Policy’ value in the 
                   [Configuration] section of the file.	Whatever policy 
                   specified will be followed.
5. Any requests not filtered by the sample provider will then be passed to 
   the next registered provider in the ordered list


Example that will let only the specified devices go to the next provider:
[Configuration]
Policy = 0                    

[Devices]
00000000000000000000FFEEDDCCBBAA=1
000000000000000000001234567890AB=1


Example that will deny service to the specified devices:
[Configuration]
Policy = 1
                         
[Devices]
00000000000000000000FFEEDDCCBBAA=0
000000000000000000001234567890AB=0

/////////////////////////////////////////////////////////////////////////////
