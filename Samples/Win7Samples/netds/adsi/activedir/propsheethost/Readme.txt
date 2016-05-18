Description
===================
The PropSheetHost sample demonstrates how to host the Active Directory Users 
and Computers MMC snap-in property sheet for an Active Directory object.

Sample Files
===================
DataObj.cpp
Main.cpp
PropSheetHost.cpp
PropSheetHost.h
StdAfx.cpp
StdAfx.h
PropSheetHost.sln
PropSheetHost.vcproj
Makefile
Readme.txt

Building the Sample
===================
The PropSheetHost sample can be built from the Platform SDK build environment 
by changing to the sample directory and running the following command from the 
command line:

    nmake

The PropSheetHost sample can also be built and run using Visual Studio .NET by 
opening the PropSheetHost.sln solution file in Visual Studio .NET.

How the Sample Works
==================== 
The PropSheetHost sample demonstrates how to host the Active Directory Users 
and Computers MMC snap-in property sheet for an Active Directory object. The 
sample implements an IDataObject that provides the data necessary to host the 
property sheet by supplying the CFSTR_DSOBJECTNAMES, CFSTR_DSDISPLAYSPECOPTIONS, 
and CFSTR_DS_PROPSHEETCONFIG data formats.

The PropSheetHost sample also demonstrates how to handle the 
WM_DSA_SHEET_CREATE_NOTIFY message to create secondary property sheets as 
required. 

If a command line parameter is passed to the application, the PropSheetHost 
sample will attempt to display the property sheet for the object at the ADsPath 
in the command line parameter. If no command line parameter is passed, the user 
will be prompted for the ADsPath of the object to display the property sheet for.

See Also
===================
CLSID_DsPropertyPages
IShellExtInit
IShellPropSheetExt
CFSTR_DSOBJECTNAMES
DSOBJECTNAMES
CFSTR_DSDISPLAYSPECOPTIONS
DSDISPLAYSPECOPTIONS
CFSTR_DS_PROPSHEETCONFIG
PROPSHEETCFG
WM_ADSPROP_NOTIFY_CHANGE
WM_DSA_SHEET_CREATE_NOTIFY
WM_DSA_SHEET_CLOSE_NOTIFY

