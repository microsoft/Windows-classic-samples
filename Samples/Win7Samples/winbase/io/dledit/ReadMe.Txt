Assign Drive Letter Sample
==========================

Summary
=======
This sample consists of a command-line tool to create, remove, and show drive
letter assignments on Windows 2000, Windows XP, and Windows Server 2003.  This
sample program creates both temporary and persistent drive letter assignments.
Temporary drive letter assignments remain valid only until the system is
shut down or rebooted; persistent assignments remain valid after shutdowns
and reboots.

This code is useful when you want to change the drive letter that is assigned
to an NT device such as a hard disk partition or a CD-ROM drive.  It is also
useful to assign a drive letter to a newly-formatted hard disk partition

Aside from _tmain(), which contains the main program there are four reusable
functions that do the main work of the sample:

   AssignPersistentDriveLetter
   RemovePersistentDriveLetter
   AssignTemporaryDriveLetter
   RemoveTemporaryDriveLetter

This sample compiles for both ANSI and UNICODE; the UNICODE build is the
default.

API functions demonstrated
==========================
   QueryDosDevice
   DefineDosDevice
   GetVolumeNameForVolumeMountPoint
   SetVolumeMountPoint
   DeleteVolumeMountPoint
   DeviceIoControl -- IOCTL_VOLUME_GET_GPT_ATTRIBUTES
   DeviceIoControl -- IOCTL_DISK_GET_PARTITION_INFO
   IsRecognizedPartition macro
   VerifyVersionInfo


Code Techniques
===============
The Windows 2000 and XP Mount Manager tracks persistent drive letter to device
assignments.  It permits one drive letter assignment per NT device.  Thus, if
you want to change an existing drive's or partition's letter, you must first
delete the current letter and then you can assign the new letter.

Applications must use the Volume Mount Point API to make or remove persistent
drive letter assignments to devices.  It should be noted that the volume mount
point APIs all require drive letters to have a colon and trailing backslash
(e.g. X:\).

When Windows first sees a volume, it writes a unique volume name in the format
\\?\Volume\{GUID}.  This uniquely identifies the volume as long as it is not
reformated, even if it is moved from one machine to another.  This volume name
is required to map a persistent drive letter.

Temporary drive letters may be assigned and removed by calling DefineDosDevice.
These drive letters are also known as "Symbolic Links" because they reside in
a part the Object Manager's namespace that is visible to user-mode programs,
but point to device names in other parts of the Object Manager's namespace that
are usable only by kernel-mode software.

While DefineDosDevice can make or remove a drive letter assignment, it lasts
only until the machine is rebooted.  DefineDosDevice requires that only the
drive letter and colon be present; the trailing backslash must not be included.

Hidden partitions and volumes are meant to be invisible to users.  This is
useful for configurations with multiple different operating systems that all
boot from the same hard disk.  All bootable partitions except one can be
hidden by a boot manager program to allow a user to select which operating
system to boot.  The behavior of hidden partitions changed from Windows 2000
to Windows XP, and it affects drive letter assignments.

On Windows 2000, hidden partitions are not really hidden; they receive PnP
notifications for arrival and departure and are assigned unique volume names.
As a result, they can have persistent drive letters.  However, DLEDIT will not
assign persistent drive letters to hidden partitions to match the behavior of
Windows XP. It will allow the user to remove persistent drive letters already
mapped to hidden partitions and will assign temporary drive letters to hidden
partitions.

On Windows XP and Windows Server 2003, partitions that were hidden when the
system was booted are not assigned a unique volume name and do not received PnP
notifications of arrival or departure.  Thus they cannot be given persistent
drive letters until they are marked as not hidden and the system rebooted.
However, applications can map a temporary drive letter to a hidden partition
to access it if needed.  DLEDIT will allow a user to assign a temporary
drive letter to a hidden partition.

The remainder of this section describes the steps to assign and remove
drive letters.


Adding a Persistent Drive Letter Assignment
-------------------------------------------
To assign a new persistent drive letter to a device, use the following steps:

1) Use DefineDosDevice to make the drive letter that refers to the NT device
   name.  This defines a symbolic link from the drive letter to the NT
   device.  At this point, applications can use the drive letter to refer
   to the device.  However, this symbolic link exists only until the machine
   is rebooted.

2) Use the newly-defined drive letter to call GetVolumeNameForVolumeMountPoint
   to get the unique volume name of the NT device. This volume name has the
   format:
      \\?\Volume{GUID}\

3) Use DefineDosDevice to delete the symbolic link created in step 1.  This
   is necessary because the Mount Manager allows only a single reference to
   the device being assigned a drive letter.

4) Call SetVolumeMountPoint with the drive letter used in step 1 and the unique
   volume name obtained in step 2.  When it completes, the drive letter will
   be assigned to the drive.  This assignment will remain even when the
   machine is rebooted.

Removing a Persistent Drive Letter Assignment
---------------------------------------------
To delete a persistent drive letter assignment, call DeleteVolumeMountPoint
with the drive letter to remove.


Adding a Temporary Drive Letter Assignment
-------------------------------------------
To assign a new temporary drive letter to a device, call DefineDosDevice.
Generally, it is good to make sure that the drive letter isn't already in use
by first calling QueryDosDevice.  Once the assignment is made, it is good to
verify that the target device exists by trying to open the drive letter with
CreateFile.


Removing a Temporary Drive Letter Assignment
---------------------------------------------
To delete a temporary drive letter assignment, call DefineDosDevice with
the drive letter to remove.


Using DLEDIT
============
How to use the utility:
   dledit <Drive Letter> <NT device name>     Add a drive letter
   dledit -t <Drive Letter> <NT device name>  Add a temporary drive letter
   dledit -r <Drive Letter>                   Remove a drive letter
   dledit <Drive Letter>                      Show mapping for drive letter
   dledit -a                                  Show all mappings


Example usage:
   dledit e:\ \Device\CdRom0
   dledit -r f:\

This utility runs on Windows 2000 and later versions because it uses the
Volume Mount Point API, which is available only on Windows 2000 and later.

To run this utility, you must be logged on as a member of the Administrators
group.  If you are logged on as any other type of user, you'll get an access
denied error (5).

Building DLEDIT
===============

Use nmake from the command line.  The makefile for this sample creates
DLEDIT.EXE.


THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1999-2001 Microsoft Corporation.  All rights reserved.

