This directory contains the source files for a sample Extensible 
Authentication Protocol (EAP) DLL.

ceapcfg.cpp
ceapcfg.h
ceapcfg.rgs
eap.c
eap.def
eap.h
eap.rc
exports.cpp
resource.h

This sample requires ATL v2.1 or ATL v3.0.

To run the sample EAP DLL, do the following:

On the server:

1) run "regsvr32 eap.dll"
2) In the Routing and RemoteAccess Properties, check EAP in the Security tab.
3) Check "Grant Remote Access Permission" in the Remote Access Policy.
4) Edit the profile, check EAP and choose the Sample EAP.
5) Reboot.

On the client:

1) run "regsvr32 eap.dll"
2) Reboot.

For more information about developing for EAP on Windows, see the SDK documentation and the raseapif.h header file.


