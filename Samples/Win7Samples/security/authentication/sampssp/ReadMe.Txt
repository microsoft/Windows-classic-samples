
SAMPSSP

Some of the APIs used in this sample are not Win32 APIs.  They are
specific APIs for Security Providers.

This sample demonstrates the implementation of a Windows NT
Security Support Provider. Security Support Providers (SSPs) are 
DLLs that implement one or more security protocols for authentication
and data security. Several SSPs are provided with Microsoft 
Operating Systems that allow applications to use the most common
security protocols (NTLM, Kerberos, SSL, etc) without having to 
implement their own provider.

If an application developer needs to implement his own security
protocol, than they may want to leverage the SSP Interface (SSPI)
and write their own Provider. The developer should carefully review
the documentation on the Platform SDK for SSPI and SSPs before
attempting to implement their own Provider.

NOTE: This sample does not use any kind of "real" credentials, or
implement a certain security protocol. The purpose of the sample
is merely to demonstrate the interface, prove the concept, and 
possibly to serve as a framework for a new package. It is expected 
that a real provider would create\provide credentials based on 
some security framework not implemented by Windows NT.

This authentication package can be used with the SOCKAUTH sample
with some minor modifications so that it requests this 
authentication package specifically.

To invoke the sample package, copy the DLL to the %systemroot%\system32
directory and make the following changes to the SOCKAUTH security.c
file:

// Make security.c unicode (before the #includes)
#define UNICODE

// replace the #defines with: 
wchar_t NT_DLL_NAME[12] = L"sampssp.dll";
wchar_t PACKAGE_NAME[12] = L"SAMPSSP";
wchar_t TOKEN_SOURCE_NAME[12] = L"AuthSamp";

// In InitPackage function
   pInit = GetProcAddress (g_hLib, "InitSecurityInterfaceW");


