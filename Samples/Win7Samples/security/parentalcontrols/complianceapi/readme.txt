Readme - Windows Vista Parental Controls Compliance API Sample

DESCRIPTION:
The Windows Vista Parental Controls infrastructure provides a simple 
COM-based API for reading the most important state values for a user's
restrictions and activity monitoring policies.

This command line application sample demonstrates code required to get each
of the available state values noted:
1.  Whether activity logging is required for the user
2.  The last settings change time for the user
3.  Flags for active policies:
	Log activity monitoring required (duplicates #1 above)
	Web restrictions on
	Time restrictions on
	Games restrictions on
	Application restrictions on
4.  For games restrictions, obtain the reason a specified game ID would be blocked
	(choices:  not blocked, internal error, explicitly set, blocked by
	 rating, blocked by rating descriptor)
5.  For web restrictions, obtain whether blocked by no-file-download policy
In addition for web restrictions, a method is exposed to trigger presenting a
dialog with a URL and optionally associated subURLs for an administrator to 
allow if acceptable.
	

PLATFORM:
The Parental Controls runtimes require Windows Vista, and are only deployed
on consumer (non-business) SKUs.  Programmatic detection of supported SKUs is
provided in the documentation.

FILES:
ComplianceAPI.cpp 		Implementation file
ComplianceAPI.h			Header
ComplianceAPI.sln		Solution file
ComplianceAPI.vcproj		Project file

The project has a dependency on the Utilities.lib static library project in 
the Utilities peer directory.

REQUIREMENTS:
No ATL or MFC dependencies are present.  The sample may be built with either
the Windows SDK environment or Visual C++. 


BUILD INSTRUCTIONS:
To build with the Windows SDK:
1.  From the Start->All Programs menu choose Microsoft Windows SDK -> CMD Shell
2.  In the WinSDK CMD Shell, navigate to: 
	Samples/Security/ParentalControls/ComplianceApi
3.  Type "vcbuild ComplianceAPI.sln"
This creates the executable in the output subdirectory for the current build
configuration(s).

KEY APIS USED IN THE SAMPLE:
COM CoCreateInstance()
WPC IWindowsParentalControls::GetUserSettings()
WPC IWPCSettings::IsLoggingRequired()
WPC IWPCSettings::GetLastSettingsChangeTime()
WPC IWPCSettings::GetRestrictions()
WPC IWindowsParentalControls::GetGamesSettings()
WPC IWPCGamesSettings::IsBlocked()
WPC IWindowsParentalControls::GetWebSettings()
WPC IWPCWebSettings::GetSettings()
WPC IWPCWebSettings::RequestURLOverride()

USAGE:

complianceapi [<username>] <function> [additional arguments]

(Note:  If username is not specified, report is for current user)

where function, additional arguments, and descriptions are as follows:

user				
-> show settings for user

isgameblocked <app ID GUID>
-> show if and why blocked for specified game ID

requrloverride <URL> [<subURL1> ... <subURLn>] 
-> request URL override (and associated subURLs if needed), showing changed status response


Returns:  0 on success, 1 on failure

DOCUMENTATION:
Refer to the Parental Controls for Windows Vista topic under the Security node.

