The source code in this directory can be used to build a sample Exit Module
for Microsoft Certificate Services.  It is meant to run on Windows 2000 or
later.  Certificate Services must already be installed.

Certificate Services calls the Exit Module through the ICertExit interface,
and the Exit Module can call back to Certificate Services through the
ICertServerExit interface.

Each time Certificate Services issues a certificate, it passes control to
the ICertExit::Notify method in exit.cpp, specifying that a certificate
has been issued.  The passed Context parameter is used with the ICertServerExit
interface to retrieve properties from the newly issued certificate.

Once the certxsam.dll DLL is built, its COM interface must be registered
via the following command:
    regsvr32 certxsam.dll

Once registered, the Certification Authority management console snapin can
be used to make this exit module active.

The Certificate Services service must be stopped and restarted to load
the newly registered Exit Module.  Use the Control Panel's Services applet,
and stop and restart the "Certificate Services" service.


Files:
------
ceerror.cpp  -- Implements error handling routines

celib.cpp    -- Implements support routines

certxsam.cpp -- Implements COM and initialization entry points:
			DllMain
			DllCanUnloadNow
			DllGetClassObject
			DllRegisterServer
			DllUnregisterServer

certxsam.def -- Exports COM entry points

certxsam.idl -- Interface Definitions

certxsam.rc  -- Version Resource

exit.cpp     -- Implements ICertExit

exit.h       -- Implements ICertExit

module.cpp   -- Implements ICertManageModule

module.h     -- Implements ICertManageModule

pch.cpp      -- Precompiled Header file

resource.h   -- Resource ID definitions
