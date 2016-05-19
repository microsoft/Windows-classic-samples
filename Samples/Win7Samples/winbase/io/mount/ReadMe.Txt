Volume Mount Point Samples
==========================


This sample consists of a couple command-line tools to create and remove
volume mount points on NTFS volumes in Windows 2000.

mount.exe
---------

A command-line tool that mounts a drive to a subdirectory. It creates a volume
mount point on the subdirectory so that the subdirectory acts as a link to
the mounted drive.  The subdirectory to be the mount point must be empty--i.e
since it will act as a link to another location, it can't contain files or
subdirectories of its own.  File operations on the mount point, such as dir,
on the mount point actually occur on the mounted drive.

The steps to create a mount point are:

1) Get the unique volume name of the drive to be mounted by calling
   GetVolumeNameForVolumeMountPoint.  This volume name has the format:

      \\?\Volume{GUID}\

2) Call SetVolumeMountPoint with the unique volume name and the directory
   to turn into a mount point.


How to use the utility:
   mount [-o] <drive> <directory>
      -o   overwrite existing mount point on <directory>

Example usage:
   mount d: c:\mount\d
   mount -o d: \mount\d     (overwrites existing mount point if present.)

API functions demonstrated:
   GetVolumeNameForVolumeMountPoint
   SetVolumeMountPoint
   FindFirstFile to determine whether a directory is a mount point.


unmount.exe
-----------
A command-line tool that removes a volume mount point from a directory, thus
unlinking the directory from the mounted drive.

To delete a mount point, call DeleteVolumeMountPoint on the directory.


How to use the utility:
   unmount <directory>

Example usage:
   unmount \mount\d

API functions demonstrated:
   DeleteVolumeMountPoint



Building mount.exe and unmount.exe
----------------------------------

Use nmake from the command line.  The makefile for this sample creates both
mount.exe and unmount.exe.


Usage notes
-----------

Volume mount points are new to Windows 2000; therefore, these command-line
tools require Windows 2000 to run successfully.  Furthermore, since only NTFS
supports mount points, you must run these samples to mount drives to
directories on an NTFS partition.  The drives being mounted may be for any
file system, but the directories that serve as the mount points must reside
on an NTFS partition.



THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1998.  Microsoft Corporation.  All rights reserved.

