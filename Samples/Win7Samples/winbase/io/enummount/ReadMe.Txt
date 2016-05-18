Enumerate Volume Mount Point Sample
===================================

This sample enumerates all volume mount points for all NTFS volumes in
Windows 2000.  The utility does not take any command line parameters.


EnumMountPoints.exe
-------------------
This command line tool outputs the following information.
* All volumes including their unique volume name and their Dos drive names.
* All mount points located on each volume including the volume which is
   mounted at the mount point.


Building EnumMountPoints.exe
----------------------------
Use nmake from the command line.  The makefile for this sample creates
EnumMountPoints.exe


Usage notes
-----------
Volume mount points are new to Windows 2000; therefore, EnumMountPoints
requires Windows 2000 to run successfully.  Furthermore, since only NTFS
supports mount points, mount points will only ever be found on NTFS
partitions.



THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) Microsoft Corporation.  All rights reserved.
