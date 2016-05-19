Readme - Windows Vista Parental Controls Compliance Application Sample

DESCRIPTION:
The Windows Vista Parental Controls infrastructure provides a simple 
COM-based API for reading the most important state values for a user's
restrictions and activity monitoring policies.

This command line application sample demonstrates a logical flow of 
decisions based upon reporting from this API for an aware application.  
The flow for games, time restriction and general appliation restrictions
specific awareness would be similar.

If time restrictions are enabled for a user, the sample also demonstrates
monitoring for the 15 and 1 minute warning events for a pending logout of
the user.
	
PLATFORM:
The Parental Controls runtimes require Windows Vista, and are only deployed
on consumer (non-business) SKUs.  Programmatic detection of supported SKUs is
provided in the documentation.

FILES:
ComplianceApp.cpp 		Implementation file
ComplianceApp.h			Header
ComplianceApp.sln		Solution file
ComplianceApp.vcproj		Project file

The project has a dependency on the Utilities.lib static library project in 
the Utilities peer directory.

REQUIREMENTS:
No ATL or MFC dependencies are present.  The sample may be built with either
the Windows SDK environment or Visual C++. 


BUILD INSTRUCTIONS:
To build with the Windows SDK:
1.  From the Start->All Programs menu choose Microsoft Windows SDK -> CMD Shell
2.  In the WinSDK CMD Shell, navigate to: 
	Samples/Security/ParentalControls/ComplianceApp
3.  Type "vcbuild ComplianceApp.sln"
This creates the executable in the output subdirectory for the current build
configuration(s).

KEY APIS USED IN THE SAMPLE:
COM CoCreateInstance()
WPC IWindowsParentalControls::GetUserSettings()
WPC IWPCSettings::IsLoggingRequired()
WPC IWPCSettings::GetRestrictions()


USAGE:

complianceapp

Returns:  0 on success, 1 on failure

Note:  If time restrictions are enabled for a user, and a logout hour has been
configured as imminent, the sample will spin and show messages for detection of
the 15 and 1 minute warning event.


DOCUMENTATION:
Refer to the Parental Controls for Windows Vista topic under the Security node.

