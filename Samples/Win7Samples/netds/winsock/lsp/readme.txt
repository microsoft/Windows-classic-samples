

INTRODUCTION

This code sample illustrates a Winsock 2 layered service provider (LSP).
An LSP is a method of inserting a layer between the Winsock 2 API and
applications. This is achieved by creating a DLL which implements the 
entire set of Winsock provider functions. These functions begin with
the three letters WSP. You can consult the Platform SDK documentation
for the specifics on these functions.


LAYERED SERVICE PROVIDERS

There are two types of LSPs. The first type is known as a base provider.
A base provider exposes access to a protocol (such as TCP/IP). Not only
does it require a user mode DLL which implements the WSP functions but
it also requires a kernel mode component which communicates with the
underlying protocol driver (usually via the TDI interface - consult
the DDK for TDI information).

The other type of LSP is the type we mentioned in the first paragraph.
It is a layer that lies above a base provider which is what this sample
is. You can install this LSP above any installed base provider. Microsoft
operating systems ship with a variety of base providers such as 
'MSAFD Tcpip [TCP/IP]' (the TCP/IP provider). You can even install this
LSP over other installed LSPs.


INSTALLING THE LSP

When you build the sample from the Samples\NetDS\WinSock\LSP folder, 
three files are generated. Instlsp.exe, ifslsp.dll and nonifslsp.dll. 
The first file is the installation program for installing the LSP into 
the Winsock catalog. The ifslsp.dll is an IFS LSP (LSP that creates sockets 
as installable file system (IFS) handles). The nonifslsp.dll is a NON-IFS LSP.
The DLLs should be copied to %SYSTEMROOT%\System32 before installation.

You need to determine which providers you want to install the LSP over
before beginning. To get a list of the installed providers, execute
the following command:

    C:\>instlsp.exe -p
    1001 - MSAFD ATM AAL5
    1002 - MSAFD Tcpip [TCP/IP]
    1003 - MSAFD Tcpip [UDP/IP]
    1004 - MSAFD Tcpip [RAW/IP]
    1005 - RSVP UDP Service Provider
    1006 - RSVP TCP Service Provider
    1007 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{74427F6E-EAED-4823-A03A-3759EBBB760B}] SEQPACKET 0
    1008 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{74427F6E-EAED-4823-A03A-3759EBBB760B}] DATAGRAM 0
    1009 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{E62BE309-3888-44C2-9A27-D981A3EB7EF5}] SEQPACKET 1
    1010 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{E62BE309-3888-44C2-9A27-D981A3EB7EF5}] DATAGRAM 1
    1011 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{A4BCF3EF-6D3E-46D0-93EF-6EBAF4E7F4FD}] SEQPACKET 2
    1012 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{A4BCF3EF-6D3E-46D0-93EF-6EBAF4E7F4FD}] DATAGRAM 2
    1013 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{4B4CCD46-9338-45DE-954C-3A379C17574A}] SEQPACKET 3
    1014 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{4B4CCD46-9338-45DE-954C-3A379C17574A}] DATAGRAM 3
    1015 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{F16E07AD-74F0-44E9-9731-DB72B9C4DD25}] SEQPACKET 4
    1016 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{F16E07AD-74F0-44E9-9731-DB72B9C4DD25}] DATAGRAM 4
    1017 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{14A9C4D0-76AC-4E14-BC60-C8DE25746F94}] SEQPACKET 5
    1018 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{14A9C4D0-76AC-4E14-BC60-C8DE25746F94}] DATAGRAM 5

This prints the current Winsock catalog. The first number is the catalog
ID followed by the string name of the corresponding Winsock provider.
This particular machine is Windows 2000 with TCP/IP, TCP/IP QOS, NetBIOS, 
and ATM providers. You will need the catalog IDs of the layers you wish
to install the LSP over. Note that if you should install the LSP over
all the providers for a given protocol family. That is if you want to 
install over the TCP provider you should also install over the UDP
and RAW providers (and probably the QOS providers as well). This is 
because there are certain applications (like Internet Explorer) which
call the function select with SOCKET handles from different providers
(UDP and TCP) which will fail if you're layered over only one of the
these two providers.

Once you've determined which providers you want to install over, you need
to perform the installation of the LSP into the Winsock catalog with the
-i option. You'll also need to specify a '-o ID' for each catalog entry
you want to layer over as well as the qualified path to the LSP DLL with '-d <DllPath>'. 
For example, to layer over all the TCP/IP providers (except QOS) you would 
execute the following command:

    C:\>instlsp.exe -i -o 1002 -o 1003 -o 1004 -n "Foobar" -d c:\windows\system32\nonifslsp.dll
    LSP name is 'Foobar'
    Installing layer: Foobar over [MSAFD Tcpip [TCP/IP]]
    Installing layer: Foobar over [MSAFD Tcpip [UDP/IP]]
    Installing layer: Foobar over [MSAFD Tcpip [RAW/IP]]

The '-n Name' option specifies the name of our LSP. You can verify the LSP
installation by calling 'instlsp.exe -p'.

    C:\>instlsp -p
    1042 - Foobar over [MSAFD Tcpip [TCP/IP]]
    1043 - Foobar over [MSAFD Tcpip [UDP/IP]]
    1044 - Foobar over [MSAFD Tcpip [RAW/IP]]
    1001 - MSAFD ATM AAL5
    1002 - MSAFD Tcpip [TCP/IP]
    1003 - MSAFD Tcpip [UDP/IP]
    1004 - MSAFD Tcpip [RAW/IP]
    1005 - RSVP UDP Service Provider
    1006 - RSVP TCP Service Provider
    1007 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{74427F6E-EAED-4823-A03A-3759EBBB760B}] SEQPACKET 0
    1008 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{74427F6E-EAED-4823-A03A-3759EBBB760B}] DATAGRAM 0
    1009 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{E62BE309-3888-44C2-9A27-D981A3EB7EF5}] SEQPACKET 1
    1010 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{E62BE309-3888-44C2-9A27-D981A3EB7EF5}] DATAGRAM 1
    1011 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{A4BCF3EF-6D3E-46D0-93EF-6EBAF4E7F4FD}] SEQPACKET 2
    1012 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{A4BCF3EF-6D3E-46D0-93EF-6EBAF4E7F4FD}] DATAGRAM 2
    1013 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{4B4CCD46-9338-45DE-954C-3A379C17574A}] SEQPACKET 3
    1014 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{4B4CCD46-9338-45DE-954C-3A379C17574A}] DATAGRAM 3
    1015 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{F16E07AD-74F0-44E9-9731-DB72B9C4DD25}] SEQPACKET 4
    1016 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{F16E07AD-74F0-44E9-9731-DB72B9C4DD25}] DATAGRAM 4
    1017 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{14A9C4D0-76AC-4E14-BC60-C8DE25746F94}] SEQPACKET 5
    1018 - MSAFD NetBIOS [\Device\NetBT_Tcpip_{14A9C4D0-76AC-4E14-BC60-C8DE25746F94}] DATAGRAM 5
    1041 - Foobar

You'll notice that for each provider that we're layering over there is a
new provider for our LSP (in this example, those entries with IDs 1042, 
1042, and 1042). Additionally there is an extra entry entitled just "Foobar".
This is the hidden (or dummy) entry for our LSP. It is required because
before we can insert our LSP into another protocol's chain we must have
a catalog ID and you don't get a catalog ID until you install a provider.
So you install a dummy provider first to obtain a valid catalog ID which you
then use in the protocol chains for the actual layered providers you install.

As a shortcut, if you wish to install the LSP over every currently installed
provider, specify -a instead of the individual '-o ID' flag, as in:

    C:\>instlsp -i -a -n "Foobar" -d c:\windows\system32\nonifslsp.dll

INSTALLING THE IFS PROVIDER

To install LSP as an IFS provider, run the installer and specify the '-h' switch (should be first
parameter to instlsp.exe) as well as the qualified path to ifslsp.dll, as in: 

    c:\>instlsp -h -i -o 1002 -o 1003 -o 1004 -n "IFSLSP" -d c:\windows\system32\ifslsp.dll

INSTALLING AN LSP ON VISTA+

Since installing an LSP requires Administrator privileges and Vista introduces
the notion of User Account Protection (UAP), installing an LSP on Vista
requires a little extra work. A manifest (instlsp.exe.manifest) has been added
to the installer code which directs Windows to prompt the non elevated user
whether the application should be allowed to execute. If invoked from an
elevated process, no prompt is given.

For more information on UAP see:
http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dnlong/html/AccProtVista.asp

Additionally, a new LSP install API has been added to Vista - WSCInstallProviderAndChains.
This function performs all the necessary steps to install an LSP in one call. It also
determines which entries need to be installed over based upon the providers supplied.
That is if you tell instlsp.exe to install only over the TCP/IPv4 entry, it will also
install over UDP/IPv4 and RAW/IPv4 since without those entries, IE (and other apps)
would not operate correctly.

INSTALLING AN LSP ON A 64-BIT OS

When installing an LSP on a 64-bit OS, both a 64-bit and 32-bit LSP DLL are needed in
order for 64-bit and 32-bit applications to access the LSPs functionality. Normally the
64-bit DLL is located in %SYSTEMROOT%\system32 and the 32-bit DLL is in %SYSTEMROOT%\SysWow64.
The OS then automagically substitutes the correct location when applications load the DLL.
However, if your application prefers to install its DLL under its own directory (which
it should), there is not automagic redirection to the 32-bit DLL. Instead when installing
your LSP, use the -d32 flag to specify the full path and filename to the 32-bit DLL. If this parameter is not specified, it is assumed that you are installing to the %SYSTEMROO%\System32
directory.

REMOVING THE LSP

Removing the sample LSP is the simplest step. Just make the following call:

    C:\>instlsp.exe -r 1041

This will remove the dummy entry as well as each layered entry. Note that 
with this sample, you cannot install the LSP more than once (that is you 
can't install the LSP over the IP providers and then try to install it
over the newly created LSP entries). This is because the LSP sample uses
a GUID to uniquely identify itself. If you attempt to install it twice, 
Winsock 2 catches the fact that a provider already exists with the given
GUID and the installation will fail. You can however create a second
instance of the DLL by changing the GUID and a few other resources. See
lspguid.cpp and/or instlsp.cpp for more information.

In the event that the Winsock catalog becomes so screwed up, you can
remove all layered entries with the following call:

    C:\>instlsp.exe -f

WINDOWS 9x AND WINDOWS ME INFORMATION

For security reasons, the LSP's DLL location is fully pathed to
%SYSTEMROOT%\system32 where it is installed. Additionally, the installer 
loads ws2_32.dll to determine whether WSCUpdateProvider is available. Both
calls will fully path the DLL location which breaks under Windows 9x and
ME. Please look at lspguid.cpp for information on compiling the sample for
use under non NT based versions of Windows.

FURTHER INFORMATION

For additional information on layered service providers consult the Platform
SDK documentation or the book "Network Programming for Microsoft Windows,
Second Edition" by Anthony Jones and Jim Ohlund (ISBN 0735615799).
