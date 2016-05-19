DskQuota    Disk quota manager.

Description
-----------
NTFS version 5.0 (included with Windows 2000) supports disk quotas.
Disk quotas allow an administrator to configure the amount of data a
particular user can store on an NTFS volume.

This sample is a console application that demonstrates how to use the
disk quota COM interfaces included with Windows 2000 to view and edit
the disk quota information for individual users and for the volume as
a whole.  

The sample uses the following COM interfaces and their methods:

   IDiskQuotaControl
      Initialize
      SetQuotaState
      GetQuotaState
      SetQuotaLogFlags
      GetQuotaLogFlags
      SetDefaultQuotaThreshold
      GetDefaultQuotaThreshold
      GetDefaultQuotaThresholdText
      SetDefaultQuotaLimit
      GetDefaultQuotaLimit
      GetDefaultQuotaLimitText
      AddUserName
      DeleteUser
      FindUserName
      CreateEnumUsers

   IDiskQuotaUser
      GetName
      GetQuotaThreshold
      GetQuotaThresholdText
      GetQuotaLimit
      GetQuotaLimitText
      GetQuotaUsed
      GetQuotaUsedText
      GetQuotaInformation
      SetQuotaThreshold
      SetQuotaLimit
      Invalidate

   IEnumDiskQuotaUsers
      Next


Descriptions of the Modules
---------------------------
DskQuota.cpp
   This is the main module.  It initializes the COM subsystem and an 
   instance of an IDiskQuotaControl COM object.

PrintMenu.cpp/PrintMenu.h
   This module implements the functions used to output the various menus.
   Each function prints out a menu, then returns an ASCII character
   representing the user's selection.

ProcessMenu.cpp/ProcessMenu.h
   This module implements the functions which takes the user's menu
   selection then decides how to act upon their choice.  Usually the
   user's selection will result in a call to a function implemented in
   the Commands.cpp module.
   
Commands.cpp/Commands.h
   This module implements the calls to the methods in the various disk
   quota COM objects.  


How to Build DskQuota
---------------------

Debug build:
   nmake

Release build:
   nmake "nodebug=1"


How to Execute DskQuota
-----------------------

DskQuota runs on Windows 2000 with the NTFS file system and can only be
run by an administrator.

Run DskQuota from the command line and specify the root directory of a
Windows 2000 NTFS volume.

Example:

   c:\>DskQuota c:\

