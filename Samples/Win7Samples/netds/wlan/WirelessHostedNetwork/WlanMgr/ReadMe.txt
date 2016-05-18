========================================================================
    STATIC LIBRARY : WlanMgr Project Overview
========================================================================

AppWizard has created this WlanMgr library project for you.

This file contains a summary of what you will find in each of the files that
make up your WlanMgr application.


station.cpp
    This is the source file that contains the CWlanStation class 
    used to provide access to information (MAC address and status, for example) on the 
    stations that are connected to the wireless Hosted Network.
    
utils.cpp
    This is the source file that contains utility functions used with WlanMgr.

utils.h
    This is the header file that contains utility functions used with WlanMgr.

WlanMgr.cpp
    This is the source file that contains the CWlanManager class 
    used to set wireless Hosted Network parameters and start and stop the Hosted Network.
    
WlanMgr.rc
    This is a listing of all of the Microsoft Windows resources that the
    WlanMgr library uses. It defines string table resources used for errors. 
    This file can be directly edited in Microsoft Visual C++. 
    
WlanMgr.vcproj
    This is the main project file for VC++ projects generated using an Application Wizard.
    This project file builds the WlanMgr static library used to initialize settings 
    for the wireless HostedNetwork application, start and stop the Hosted Network. 
    This library is used set wireless Hosted Network parameters (SSID and Key, 
    for example), query the settings of Hosted Network, start and stop the Hosted Network, 
    and  handle the notification events from Hosted network. 
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    Application Wizard.


/////////////////////////////////////////////////////////////////////////////

Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named WlanMgr.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Visual C++ reads and updates this file.

targetver.h
    This is the header file that defines the version of Windows that supports the
    programming elements used in this library. 
    Microsoft Visual C++ reads and updates this file.

/////////////////////////////////////////////////////////////////////////////

Other notes:

AppWizard uses "TODO:" comments to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////
