========================================================================
       CLUSTER ADMINISTRATOR EXTENSION : File Share Sample
========================================================================


AppWizard has created this Cluster Administrator Extension DLL for you.
This DLL demonstrates the basics of modifying the interface of
Cluster Administrator and is also a starting point for writing your DLL.

This file contains a summary of what you will find in each of the files that
make up your Cluster Administrator Extension DLL.

File Share SampleEx.h
    This is the main header file for the DLL.  It declares the
    CFileShareSampleApp class.

File Share SampleEx.cpp
    This is the main DLL source file.  It contains the class CFileShareSampleApp.
    It also contains the OLE entry points required of inproc servers.

File Share SampleEx.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes the icons, bitmaps, and cursors that are stored
    in the RES subdirectory.  This file can be directly edited in Microsoft
    Developer Studio.

res\File Share SampleEx.rc2
    This file contains resources that are not edited by Microsoft 
    Developer Studio.  You should place all resources not
    editable by the resource editor in this file.

File Share SampleEx.def
    This file contains information about the DLL that must be
    provided to run with Microsoft Windows.  It defines parameters
    such as the name and description of the DLL.  It also exports
    functions from the DLL.

File Share SampleEx.clw
    This file contains information used by ClassWizard to edit existing
    classes or add new classes.  ClassWizard also uses this file to store
    information needed to create and edit message maps and dialog data
    maps and to create prototype member functions.

ExtObj.h
    This is the header file which defines the classes which implement the
    COM interfaces by the Microsoft Windows NT Cluster Administrator program
    for adding property pages, wizard pages, or context menu items.  It
    defines the CExtObject class.

ExtObj.cpp
    This is the source file which implements the CExtObject class.

ExtObjID.idl
    This the Interface Definition Language source file for defining
    the COM object implemented by the extension DLL.  This is the object
    that will be loaded by the Cluster Administrator program.

RegExt.h
    This is the header file which declares the functions used to register
    the Cluster Administrator extension DLL with both the cluster and the
    client machine.

RegExt.cpp
    This is the source file which implements registration for the Cluster
    Administrator extension DLL.

/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named File Share Sample.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Developer Studio reads and updates this file.

/////////////////////////////////////////////////////////////////////////////
Property page/context menu files:

BasePage.h
    This is the header file which defines a class which provides base
    property page functionality for use by extension property pages.  It
    defines the CBasePropertyPage class.

BasePage.cpp
    This is the source file which implements the CBasePropertyPage class.

PropList.h
    This is the header file which defines classes for manipulating
    cluster property lists.  It defines the CClusPropList and CObjectProperty
    classes.

PropList.cpp
    This is the source file which implements the CClusPropList and
    CObjectProperty classes for manipulating cluster property lists.

ResProp.h
    This is the header file which defines a property page titled "Parameters"
    to add to property sheets for resources for which your extension DLL is
    written.  It defines the CFile Share SampleParamsPage class.

ResProp.cpp
    This is the source file which implements the CFile Share SampleParamsPage class.

ExtObjData.h
    This is the header file which defines functions for adding items to
    context menus, adding custom property sheets, and wizard pages.

ExtObjData.cpp
    This is the source file which defines functions for adding items to
    context menus, adding custom property sheets, and wizard pages.

/////////////////////////////////////////////////////////////////////////////
Other notes:

If the extension DLL extends a resource type (by default it does) then before
registering the extension DLL the resource type needs to be created.  If the
resource type has not been created then the registration command will return the
following:

    System error 5078 has occurred (0x000013d6).
    The specified resource type was not found.

To fix this, create the resource type as detailed in the resource DLL project's
ReadMe.txt and try the registration again.

To register your extension DLL so that it can be used with Cluster
Administrator (CluAdmin), copy DLL to each machine that Cluadmin will be run on
to administer the cluster and register the DLL with regsvr32.exe:

    regsvr32 "File Share SampleEx.dll"

It's recommended that the DLL be copied to each node of the cluster and
registered there as well.  Then from a machine with the DLL present enter the
following command:

    cluster [clustername] /RegAdminExt:"File Share SampleEx.dll"

If the machine is not a member of the cluster then the "[clustername]" portion
is required.

"TODO:" indicates a part of the source code you should add to or customize.

"NOTE:" indicates a point of interest in the source code.

This DLL requires that the MFC libraries be installed with Visual Studio or
you will encounter compile errors.


/////////////////////////////////////////////////////////////////////////////
