Readme - Windows Vista Parental Controls Web Extensibility Sample

DESCRIPTION:
The Windows Vista Parental Controls infrastructure provides an extensibility
mechanism for ISV components to register an alternate web content filter 
overriding the filter provided with Vista, as only one filter should be 
active at any time.

This command line application sample demonstrates code required to get and 
set the FilterID and FilterNamePath properties necessary for registration.  It 
also exposes management of two exemption lists for overriding filtering for
entire applications or URLs matching specified strings to the length of 
those strings.

Note that the FilterID GUID used for a web content filter override must have
been registered as a User Interface extensibility link as shown in the 
UIExtensibility sample.  Setting the FilterID property to the same GUID as 
for the registered UI link will promote the link from a generic UI extension
to the exclusive active web content filter position.  Resetting this override
requires returning the FilterID to the NULL_GUID value and the FilterNamePath
to a null variant.

The FilternamePath is a string consisting of a path to a resource DLL with ","
and a negative resource ID appended.  This promotes localized binary deployments.

PLATFORM:
The Parental Controls runtimes require Windows Vista, and are only deployed
on consumer (non-business) SKUs.  Programmatic detection of supported SKUs is
provided in the documentation.

FILES:
WebExtensibility.cpp 		Implementation file
WebExtensibility.h		Header
WebExtensibility.sln		Solution file
WebExtensibility.vcproj		Project file

The project has a dependency on the Utilities.lib static library project in 
the Utilities peer directory.

REQUIREMENTS:
No ATL or MFC dependencies are present.  The sample may be built with either
the Windows SDK environment or Visual C++. 


BUILD INSTRUCTIONS:
To build with the Windows SDK:
1.  From the Start->All Programs menu choose Microsoft Windows SDK -> CMD Shell
2.  In the WinSDK CMD Shell, navigate to: 
	Samples/Security/ParentalControls/WebExtensibility
3.  Type "vcbuild WebExtensibility.sln"
This creates the executable in the output subdirectory for the current build
configuration(s).

KEY APIS USED IN THE SAMPLE:
WMI IWbemServices::GetObject()
WMI IWbemServices::PutInstance()

USAGE:

webextensibility <operation> [argument1..n]

where operations, associated arguments,and descriptions are as follows:

http list				
-> list system-wide exemption paths for http-based applications

http add <path>
-> add an application exemption path to the list

http delete <path>
-> delete the specified application exemption path from the list

url list
-> list strings to be matched to appropriate substring length in a URL

url add <URL>
-> add string to URL list

url delete <URL>
-> remove string from URL list

filter get
-> display current filter ID and name path.  ID composed of 0's and NULL name 
   path denote the Windows Vista in-box filter

filter set /g:<FilterID> /n:<FilterNamePath>
-> set overriding filter by specifying ID and name path.  This is used in 
   conjunction with the UIExtensibility sample link registration to have a 
   web content filter link displayed at the top of the More Settings region

filter reset
-> return filter ID and name path to settings enabling the Windows Vista provided
   web content filter for the user

Returns:  0 on success, 1 on failure


DOCUMENTATION:
Refer to the Parental Controls for Windows Vista topic under the Security node.

