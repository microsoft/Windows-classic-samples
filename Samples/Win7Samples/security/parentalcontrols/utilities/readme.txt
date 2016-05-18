Readme - Windows Vista Parental Controls Utilities

DESCRIPTION:
The Windows Vista Parental Controls infrastructure supports two types of APIs.  
One is a simple,COM-based API named the Compliance API. This API allows for 
read-only access to key configuration settings.  A second API makes use of a 
Windows Management Instrumentation (WMI) provider to allow for read/write access 
to all exposed configuration settings of Parental Controls.  

This sample builds a static library of basic COM initialization, WMI helper, 
and SID access functions that are used in most of the Parental Controls
samples. These functions enhance readability and reduce duplication of code
for the other samples.

Functions exposed by this library are prepended with "Wpcu" to simplify 
recognition as Windows Parental Controls Utility functions.

PLATFORM:
The Parental Controls runtimes require Windows Vista, and are only deployed
on consumer (non-business) SKUs.  Programmatic detection of supported SKUs is
provided in the documentation.

FILES:
Utilities.cpp		Implementation file
Utilities.h		Header
Utilities.sln		Solution file
Utilities.vcproj	Project file

REQUIREMENTS:
No ATL or MFC dependencies are present.  The sample may be built with either
the Windows SDK environment or Visual C++.

BUILD INSTRUCTIONS:
The library may be built directly using the solution file in this directory, but
this is not necessary.  The project has been added as a dependency to all sample
solutions requiring linking with the library.

To build with the Windows SDK:
1.  From the Start->All Programs menu choose Microsoft Windows SDK -> CMD Shell
2.  In the WinSDK CMD Shell, navigate to: 
	Samples/Security/ParentalControls/Utilities
3.  Type "vcbuild Utilities.sln"
This creates the executable in the output subdirectory for the current build
configuration(s).

KEY APIS USED IN THE LIBRARY:
COM CoInitialize()
COM CoInitializeSecurity()
COM CoUninitialize()
WMI IWbemLocator::ConnectServer()
WMI IWbemClassObject::Put()
WMI IWbemClassObject::Get()

Note that encapsulating IWbemClassObject::Put() with IWbemServices::PutInstance() 
here was considered non-optimal as some WMI class objects have multiple properties 
requiring Put() and only needing one final commit via IWbemClassObject::PutInstance().


DOCUMENTATION:
Please refer to the Parental Controls for Windows Vista topic under the Security 
node.

