================================================================================
    MICROSOFT FOUNDATION CLASS LIBRARY : HostedNetwork Project Overview
===============================================================================

The application wizard has created this HostedNetwork application for
you.  This application not only demonstrates the basics of using the Microsoft
Foundation Classes but is also a starting point for writing your application.

This file contains a summary of what you will find in each of the files that
make up your HostedNetwork application.

device.cpp
    This is the source file that contains the CWlanDevice class used for wireless
    LAN device interformation.

device.h
    This is the header file for wireless LAN device information. It declares the
    CWlanDevice application class.

HostedNetwork.cpp
    This is the main application source file that contains the application
    class CHostedNetworkApp.

HostedNetwork.h
    This is the main header file for the application.  It includes other
    project specific headers (including Resource.h) and declares the
    CHostedNetworkApp application class.

HostedNetwork.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes the icons, bitmaps, and cursors that are stored
    in the RES subdirectory.  This file can be directly edited in Microsoft
    Visual C++. 

HostedNetwork.vcproj
    This is the main project file for VC++ projects generated using an application wizard. 
    This project file builds the main HostedNetwork application. It contains information
    about the version of Visual C++ that generated the file, and information about the 
    platforms, configurations, and project features selected with the application wizard.

HostedNetworkDlg.cpp
    This is the source file that contains the CHostedNetworkDlg class used by dialogs
    for the user interface.

HostedNetworkDlg.h
    This is the header file used by dialogs for the user interface. It declares the
    CHostedNetworkDlg application class.

notif.cpp
    This is the source file that contains the CHostedNetworkNotification class 
    used to pass notifications from the HostedNetwork application to the user 
    interface.

notif.h
    This is the header file used to pass notifications from the HostedNetwork
    application to the user interface. It declares the
    CHostedNetworkNotification application class.

res\Camera.ico
    This is an icon file used for a camera connected to the HostedNetwork application.  
    This icon is included by the main resource file HostedNetwork.rc.

res\computer.ico
    This is an icon file used for a computer connected to the HostedNetwork application.  
    This icon is included by the main resource file HostedNetwork.rc.

res\DefaultDevice.ico
    This is an icon file used for the default device connected to the HostedNetwork application.  
    This icon is included by the main resource file HostedNetwork.rc.

res\HostedNetwork.ico
    This is an icon file, which is used as the application's icon.  This
    icon is included by the main resource file HostedNetwork.rc.

res\HostedNetwork.rc2
    This file contains resources that are not edited by Microsoft
    Visual C++. You should place all resources not editable by
    the resource editor in this file.

res\Printer.ico
    This is an icon file used for a printer connected to the HostedNetwork application.  
    This icon is included by the main resource file HostedNetwork.rc.

res\Telephone.ico
    This is an icon file used for a mobile telephone connected to the HostedNetwork application.  
    This icon is included by the main resource file HostedNetwork.rc.

res\ZuneDevices.ico
    This is an icon file used for a Zune device connected to the HostedNetwork application.  
    This icon is included by the main resource file HostedNetwork.rc.


/////////////////////////////////////////////////////////////////////////////

The application wizard creates one dialog class:

HostedNetworkDlg.h, HostedNetworkDlg.cpp - the dialog
    These files contain your CHostedNetworkDlg class.  This class defines
    the behavior of your application's main dialog.  The dialog's template is
    in HostedNetwork.rc, which can be edited in Microsoft Visual C++.


/////////////////////////////////////////////////////////////////////////////

Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named HostedNetwork.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Visual C++ reads and updates this file.

targetver.h
    This is the header file that defines the version of Windows that supports the
    programming elements used in this application. 
    Microsoft Visual C++ reads and updates this file.

/////////////////////////////////////////////////////////////////////////////

Other notes:

The application wizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

If your application uses MFC in a shared DLL, you will need
to redistribute the MFC DLLs. If your application is in a language
other than the operating system's locale, you will also have to
redistribute the corresponding localized resources MFC90XXX.DLL.
For more information on both of these topics, please see the section on
redistributing Visual C++ applications in MSDN documentation.

/////////////////////////////////////////////////////////////////////////////
