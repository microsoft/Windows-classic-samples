THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 2002  Microsoft Corporation.  All Rights Reserved.

Windows 2000 RAS Custom Dial DLL Sample
---------------------------------------

Abstract:

Windows 2000 implements the AutoDial feature of RAS in a slightly different
manner then previous versions of Windows did. Windows 2000 extends the RASENTRY
structure to provide a pointer to a custom dial dll that greatly expands the
possibilities of this function for applications running on Windows 2000. The
application now can provide it's own dialing and entry dialogs in addition to
performing the custom dial features all within the normal context of a RAS
phonebook entry. This feature set provides a much more expanded functionality 
then what was present in the AutoDial feature of previous versions of Windows.

The unfortunate consequence of this expanded functionality is that Windows 2000
does not support the old AutoDial entry in the RASENTRY structure and thus 
Windows 2000 applications that need to provide similar functionality need to 
implement functionality that this sample illustrates. The trade-offs are a much
better designed set of functionality and customization available while retaining
the the familiar look-and-feel of the Windows 2000 Dial-up components. 

Supported OS:

  Windows 2000, Windows XP

Building:

  Build the sample using the latest Platform SDK via the MAKEFILE included.
  When using the RAS API in an application it must link with rasapi32.lib. 
  Also, since this sample uses extensions to RAS unique to Windows 2000, 
  the variable WINVER needs to be set to 0x0500 prior to the statements that
  include the RAS function and structure declarations.

Installation/Uninstallation:

  Once the DLL has been built, a new phone entry needs to be created that contains
  a path to this .DLL. You will need to also buid 
  <Platform SDK Root>\Samples\NetDs\RAS\CustomScript\CustomEntry 
  sample. It will let you create a custom entry with a path to this .DLL.

Usage:

  Once you create a phone entry and set the customDLL path to point to this DLL
  you will be able to dial using that entry. Simply using this entry will cause
  the DLL to be used when the underlying system calls RasDial(), RasDialDlg(),
  RasHangup(), RasEntryDlg(), or RasDeleteEntry() on this entry. Each of these
  calls are then forwarded through the customdial DLL that is associated with 
  that phonebook entry. One can use the Windows 2000 user interface or the 
  sample programs in the Platform SDK (in the 
  <Platform SDK Root>\Samples\NetDs\RAS folder) to see the effects the DLL has. 
  This DLL simply provides a basic and generic implementation that shows how 
  the DLL should be setup for custom dialing on an entry.

  This sample has been tested for most common situations, however, it is not
  designed to be deployed as-is. This is "sample" quality and as such simply
  shows the techniques needed to provide an implementation in the most
  common scenarios. As in any software development project, testing should
  be done to catch all potential problems that could arise.

Debugging:

  The Custom Dial DLL Sample uses the OutputDebugString() API to send 
  strings to an attached debugger. There are several programs that can capture
  this information and display it while the DLL is in use. This can be a 
  rudimentary technique for debugging the DLL while it is being used. The 
  Platform SDK contains one such program capable of displaying this information 
  called DBMON.EXE. Simply run DBMON.EXE before using the DLL to see the DLL's
  output.
