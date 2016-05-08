Readme - Windows Vista Parental Controls User Interface Extensibility Sample

DESCRIPTION:
The Windows Vista Parental Controls infrastructure provides an extensibility
mechanism for ISV components to register launch links for display on the
central control panel.  Via WMI, the API allows specification of an icon path,
disabled icon path, main text name resource path, and subtitle text resource 
path to show, and an executable path to invoke when the "link" rectangle is 
clicked.

This command line application sample demonstrates code required to list all 
registered links, and query for links meeting a WQL specification.  It also 
shows adding, modifying, and deleting links tagged by GUID as would be 
necessary for real-world software deployment, updates, and uninstallation.
Code implementing each operation is wrapped in a Wpcs (Windows Parental
Controls Sample) prepended function to facilitate experimentation and trial 
integration.

PLATFORM:
The Parental Controls runtimes require Windows Vista, and are only deployed
on consumer (non-business) SKUs.  Programmatic detection of supported SKUs is
provided in the documentation.

FILES:
UIExtensibility.cpp 		Implementation file
UIExtensibility.h		Header
UIExtensibility.sln		Solution file
UIExtensibility.vcproj		Project file

The project has a dependency on the Utilities.lib static library project in 
the Utilities peer directory.

REQUIREMENTS:
No ATL or MFC dependencies are present.  The sample may be built with either
the Windows SDK environment or Visual C++. 


BUILD INSTRUCTIONS:
To build with the Windows SDK:
1.  From the Start->All Programs menu choose Microsoft Windows SDK -> CMD Shell
2.  In the WinSDK CMD Shell, navigate to: 
	Samples/Security/ParentalControls/UIExtensibility
3.  Type "vcbuild UIExtensibility.sln"
This creates the executable in the output subdirectory for the current build
configuration(s).

KEY APIS USED IN THE SAMPLE:
WMI IWbemServices::CreateInstanceEnum()
WMI IEnumWbemClassObject::Next()
WMI IWbemServices::ExecQuery()
WMI IWbemServices::GetObject()
WMI IWbemClassObject::SpawnInstance()
WMI IWbemServices::PutInstance()
WMI IWbemServices::DeleteInstance()

USAGE:

uiextensibility <operation> [argument1..n]

where operations, associated arguments,and descriptions are as follows:

list				
-> list registered extensions

query <query_string>		
-> perform query per WMI WQL syntax, listing matches

add /g:<GUID> /c:<subsystem> /n:<name_path> /s:<subtitle_path> /i:<image_path> /d:<disabled_image_path> /e:<exe_path>
-> add new extension keyed by GUID string and subsystem number (currently always 0)

mod /g:<GUID> /c:<subsystem> [/n:<name> /s:<subtitle> /i:<image_path> /d:<disabled_image_path> /e:<exe_path>]
-> modify one or more elements in existing extension, using key of GUID string and subsystem

del /g:<GUILD> /c:<subsystem>
-> delete extension keyed by GUID string and subsystem number

Notes: 
 1.  The subsystem argument is currently always 0.  It is present for future growth
 2.  The name_path, subtitle_path, image_path and disabled_image_path arguments 
     conform to Shell Extensibility standards for a negative resource number in the 
     binary.  As an example, using the WPC control panel DLL a usable image_path 
     would be c:/windows/system32/wpccpl.dll,-20.

Returns:  0 on success, 1 on failure

DOCUMENTATION:
Refer to the Parental Controls for Windows Vista topic under the Security node.

