MAPNAME.DLL
Sample extension DLL for the Internet Authentication Service (IAS).


Overview
--------

Normally, if a RADIUS User-Name does not contain a domain name, IAS assumes the
account exists in the domain of the IAS server. This sample extension DLL
instead searches all trusted domains for the designated account. This allows
users from multiple domains to be authenticated without the users having to
supply their domain name.


What this sample demonstrates from a SDK perspective
----------------------------------------------------

This sample demonstrates how to modify RADIUS attributes. In this sample the
request is modified before the access request is processed by the RADIUS
server. The sample is registered as an "extension plug-in", which gets invoked
before user authentication.

You can also modify other RADIUS packets before or after they are processed by
the IAS server. Please refer to SDK help for more details on the types of
RADIUS packets that can be modified and the stages at which they can be
modified.


Building the sample
-------------------

To build the sample, run NMAKE from the Platform SDK build environment. Any C
compiler can be used to build the sample; Visual C++ is not required.


Running the sample
------------------

The sample will run on any of the Windows Server 2003 family.

To install the sample, execute
    regsvr32 mapname.dll
from a command-prompt and then restart IAS.

To uninstall the sample, execute
    regsvr32 /u mapname.dll
from a command-prompt and then restart IAS.


Implementation details
----------------------

The sample demonstrates the following APIs:
   RadiusExtensionInit
   RadiusExtensionTerm
   RadiusExtensionProcess2

Files:
   dllmain.c - Contains the DLL entry point and the registration functions.
   mapname.c - Contains the DLL exports for the extension.
   multisz.c - Functions for manipulating values of type REG_MULTI_SZ.
   radutil.c - Utility functions for implementing an extension DLL.
