Virtual hard disk management sample
===================================

This sample demonstrates how to use the Hyper-V WMI APIs and Virtual Hard Disk APIs to manage virtual hard disks.

The Windows-classic-samples repo includes a variety of code samples that demonstrate the use of various new programming features for managing Hyper-V that are available starting in Windows 8.1 and/or Windows Server 2012 R2. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more information about the programming models, platforms, languages, and APIs demonstrated in this sample, please refer to the [Hyper-V WMI provider (V2)](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850319) documentation.

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

Related technologies
--------------------

[Hyper-V WMI provider (V2)](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850319), [Virtual Storage](http://msdn.microsoft.com/en-us/library/windows/desktop/dd323653)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

Building the C\# version of the sample
--------------------------------------

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file titled Storage.sln.

3.  Press F7 (or F6 for Microsoft Visual Studio 2013) or use **Build** \> **Build Solution** to build the sample.

Building the C++ version of the sample
--------------------------------------

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file titled Storage.sln.

3.  Press F7 (or F6 for Visual Studio 2013) or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

Running the C\# version of the sample
-------------------------------------

**Note**  This sample must be run as an administrator.

This sample is written in C\# using the Hyper-V WMI APIs, and requires some experience with WMI programming.

This sample can be run in several different modes.

### Retrieve information about a virtual hard disk

To retrieve information about a virtual hard disk using the [**GetVirtualHardDiskSettingData**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850064) and [**GetVirtualHardDiskState**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850065) methods, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **StorageSamplesWmi.exe GetVirtualHardDisk** *ServerName* **** *VhdPath*

    where the parameters are as follows:

    -   *ServerName* is the name of the server on which to perform the operation.
    -   *VhdPath* is the path of the virtual hard disk file.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Modify the settings of a virtual hard disk

To modify the settings of a virtual hard disk using the [**SetVirtualHardDiskSettingData**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850302) method, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **StorageSamplesWmi.exe SetVirtualHardDisk** *ServerName* **** *VhdPath* **** *ParentPath* **** *Format* **** *FileSize* **** *BlockSize* **** *SectorSize*

    where the parameters are as follows:

    -   *ServerName* is the name of the server on which to perform the operation.
    -   *VhdPath* is the path of the virtual hard disk file to be modified.
    -   *ParentPath* is the new path of the parent virtual hard disk file.
    -   *Format* is "vhdx" to make the virtual hard disk a VHDX or "vhd" to make the virtual hard disk a VHD.
    -   *FileSize* is the new maximum size of the virtual hard disk, in bytes.
    -   *BlockSize* is the new block size of the virtual hard disk, in bytes.
    -   *SectorSize* is the new sector size of the virtual hard disk, in bytes.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Validate a virtual hard disk

To validate a virtual hard disk using the [**ValidateVirtualHardDisk**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850312) method, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **StorageSamplesWmi.exe ValidateVirtualHardDisk** *ServerName* **** *VhdPath*

    where the parameters are as follows:

    -   *ServerName* is the name of the server on which to perform the operation.
    -   *VhdPath* is the path of the virtual hard disk file.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Create a fixed virtual hard disk

To create a fixed virtual hard disk using the [**CreateVirtualHardDisk**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850039) method, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **StorageSamplesWmi.exe CreateFixedVirtualHardDisk** *ServerName* **** *VhdPath* **** *Format* **** *FileSize* **** *BlockSize* **** *SectorSize*

    where the parameters are as follows:

    -   *ServerName* is the name of the server on which to perform the operation.
    -   *VhdPath* is the path of the virtual hard disk file to be created.
    -   *Format* is "vhdx" to make the new virtual hard disk a VHDX or "vhd" to make the new virtual hard disk a VHD.
    -   *FileSize* is the maximum size of the virtual hard disk, in bytes.
    -   *BlockSize* is the block size of the virtual hard disk, in bytes.
    -   *SectorSize* is the sector size of the virtual hard disk, in bytes.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Create a dynamic virtual hard disk

To create a dynamic virtual hard disk using the [**CreateVirtualHardDisk**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850039) method, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **StorageSamplesWmi.exe CreateDynamicVirtualHardDisk** *ServerName* **** *VhdPath* **** *Format* **** *FileSize* **** *BlockSize* **** *SectorSize*

    where the parameters are as follows:

    -   *ServerName* is the name of the server on which to perform the operation.
    -   *VhdPath* is the path of the virtual hard disk file to be created.
    -   *Format* is "vhdx" to make the new virtual hard disk a VHDX or "vhd" to make the new virtual hard disk a VHD.
    -   *FileSize* is the initial maximum size of the virtual hard disk, in bytes.
    -   *BlockSize* is the block size of the virtual hard disk, in bytes.
    -   *SectorSize* is the sector size of the virtual hard disk, in bytes.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Create a differencing virtual hard disk

To create a differencing virtual hard disk using the [**CreateVirtualHardDisk**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850039) method, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **StorageSamplesWmi.exe CreateDifferencingVirtualHardDisk** *ServerName* **** *VhdPath* **** *ParentPath* **** *Format*

    where the parameters are as follows:

    -   *ServerName* is the name of the server on which to perform the operation.
    -   *VhdPath* is the path of the new virtual hard disk file.
    -   *ParentPath* is the path of the parent virtual hard disk file.
    -   *Format* is "vhdx" to make the new virtual hard disk a VHDX or "vhd" to make the new virtual hard disk a VHD.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Create a virtual floppy disk

To create a virtual floppy disk using the [**CreateVirtualFloppyDisk**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850038) method, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **StorageSamplesWmi.exe CreateVirtualFloppyDisk** *ServerName* **** *VhdPath*

    where the parameters are as follows:

    -   *ServerName* is the name of the server on which to perform the operation.
    -   *VhdPath* is the path of the virtual hard disk file.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Attach a virtual hard disk

To attach a virtual hard disk using the [**AttachVirtualHardDisk**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850023) method, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **StorageSamplesWmi.exe AttachVirtualHardDisk** *ServerName* **** *VhdPath* **** *AssignDriveLetter* **** *ReadOnly*

    where the parameters are as follows:

    -   *ServerName* is the name of the server on which to perform the operation.
    -   *VhdPath* is the path of the virtual hard disk file to be attached.
    -   *AssignDriveLetter* is "True" to automatically assign a drive letter to the attached drive or "False" otherwise.
    -   *ReadOnly* is "True" to make the attached drive read only or "False" otherwise.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Detach a virtual hard disk

To detach a virtual hard disk using the [**DetachVirtualHardDisk**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850046) method, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **StorageSamplesWmi.exe DetachVirtualHardDisk** *ServerName* **** *VhdPath*

    where the parameters are as follows:

    -   *ServerName* is the name of the server on which to perform the operation.
    -   *VhdPath* is the path of the virtual hard disk file.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Update the parent of a virtual hard disk

To update the parent of a virtual hard disk using the [**SetParentVirtualHardDisk**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850298) method, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **StorageSamplesWmi.exe SetParentVirtualHardDisk** *ServerName* **** *ChildPath* **** *ParentPath* **** *LeafPath* **** *IgnoreIDMistmatch*

    where the parameters are as follows:

    -   *ServerName* is the name of the server on which to perform the operation.
    -   *ChildPath* is the path of the child virtual hard disk file.
    -   *ParentPath* is the path of the parent virtual hard disk file.
    -   *LeafPath* is the path of the leaf virtual hard disk file.
    -   *IgnoreIDMistmatch* is "True" to forcibly set the parent even if the identifiers do not match, or "False" otherwise.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Convert a virtual hard disk to fixed

To convert a non-fixed virtual hard disk to fixed using the [**ConvertVirtualHardDisk**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850035) method, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **StorageSamplesWmi.exe ConvertVirtualHardDisk** *ServerName* **** *VhdSourcePath* **** *VhdDestinationPath*

    where the parameters are as follows:

    -   *ServerName* is the name of the server on which to perform the operation.
    -   *VhdSourcePath* is the path of the virtual hard disk file to be converted.
    -   *VhdDestinationPath* is the destination path of the converted virtual hard disk file.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Merge a virtual hard disk

To merge a virtual hard disk using the [**MergeVirtualHardDisk**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850091) method, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **StorageSamplesWmi.exe MergeVirtualHardDisk** *ServerName* **** *VhdSourcePath* **** *VhdDestinationPath*

    where the parameters are as follows:

    -   *ServerName* is the name of the server on which to perform the operation.
    -   *VhdSourcePath* is the path of the virtual hard disk file to be merged.
    -   *VhdDestinationPath* is the destination path of the merged virtual hard disk file.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Compact a virtual hard disk

To compact a virtual hard disk using the [**CompactVirtualHardDisk**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850033) method, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **StorageSamplesWmi.exe CompactVirtualHardDisk** *ServerName* **** *VhdPath* **** *Mode*

    where the parameters are as follows:

    -   *ServerName* is the name of the server on which to perform the operation.
    -   *VhdPath* is the path of the virtual hard disk file.
    -   *Mode* specifies the compaction mode and is one of the numeric values specified for the *Mode* parameter of the [**CompactVirtualHardDisk**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850033) method.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Resize a virtual hard disk

To resize a virtual hard disk using the [**ResizeVirtualHardDisk**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850285) method, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **StorageSamplesWmi.exe ResizeVirtualHardDisk** *ServerName* **** *VhdPath* **** *NewSize*

    where the parameters are as follows:

    -   *ServerName* is the name of the server on which to perform the operation.
    -   *VhdPath* is the path of the virtual hard disk file to be resized.
    -   *NewSize* is the new maximum size of virtual hard disk file, in bytes.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

Running the C++ version of the sample
-------------------------------------

**Note**  This sample must be run as an administrator.

This sample is written in C++ using the Virtual Storage APIs and requires some experience with Windows API programming.

The sample demonstrates how to perform each of the following operations:

### Retrieve information about a virtual hard disk

To retrieve information about a virtual hard disk using the [**GetVirtualDiskInformation**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd323670) function, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **Storage.exe GetVirtualDiskInformation** *VhdPath*

    where *VhdPath* is the path of the virtual hard disk file.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Create a fixed virtual hard disk

To create a fixed virtual hard disk using the [**CreateVirtualDisk**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd323659) function, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **Storage.exe CreateFixedVirtualDisk** *VhdPath* **** *FileSize* **** *BlockSize* **** *SectorSize*

    where the parameters are as follows:

    -   *VhdPath* is the path of the virtual hard disk file.
    -   *FileSize* is the maximum size of the virtual hard disk.
    -   *BlockSize* is the block size of the virtual hard disk, in bytes.
    -   *SectorSize* is the sector size of the virtual hard disk, in bytes.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Create a dynamic virtual hard disk

To create a dynamic virtual hard disk using the [**CreateVirtualDisk**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd323659) function, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **Storage.exe CreateDynamicVirtualDisk** *VhdPath* **** *FileSize* **** *BlockSize* **** *SectorSize*

    where the parameters are as follows:

    -   *VhdPath* is the path of the virtual hard disk file.
    -   *FileSize* is the initial maximum size of the virtual hard disk.
    -   *BlockSize* is the block size of the virtual hard disk, in bytes.
    -   *SectorSize* is the sector size of the virtual hard disk, in bytes.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Create a differencing virtual hard disk

To create a differencing virtual hard disk using the [**CreateVirtualDisk**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd323659) function, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **Storage.exe CreateDifferencingVirtualDisk** *VhdPath* **** *ParentPath*

    where the parameters are as follows:

    -   *VhdPath* is the path of the virtual hard disk file.
    -   *ParentPath* is the path of the parent virtual hard disk file.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Attach a virtual hard disk

To attach a virtual hard disk using the [**AttachVirtualDisk**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd323692) function, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **Storage.exe AttachVirtualDisk** *VhdPath* **** *ReadOnly*

    where the parameters are as follows:

    -   *VhdPath* is the path of the virtual hard disk file.
    -   *ReadOnly* is "True" to make the attached drive read only or "False" otherwise.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Detach a virtual hard disk

To detach a virtual hard disk using the [**DetachVirtualDisk**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd323696) function, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **Storage.exe DetachVirtualDisk** *VhdPath*

    where *VhdPath* is the path of the virtual hard disk file.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Update information about a virtual hard disk

To update information about a virtual hard disk, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **Storage.exe SetVirtualDiskInformation** *VhdPath* **** *ParentPath* **** *PhysicalSectorSize*

    where the parameters are as follows:

    -   *VhdPath* is the path of the virtual hard disk file.
    -   *ParentPath* is the path of the parent virtual hard disk file.
    -   *PhysicalSectorSize* is the new physical sector size, in bytes.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Merge a virtual hard disk

To merge a virtual hard disk using the [**MergeVirtualDisk**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd323676) function, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **Storage.exe MergeVirtualDisk** *VhdPath*

    where *VhdPath* is the path of the virtual hard disk file.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Compact a virtual hard disk

To compact a virtual hard disk using the [**CompactVirtualDisk**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd323655) function, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **Storage.exe CompactVirtualDisk** *VhdPath*

    where *VhdPath* is the path of the virtual hard disk file.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Resize a virtual hard disk

To resize a virtual hard disk, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **Storage.exe ResizeVirtualDisk** *VhdPath* **** *FileSize*

    where the parameters are as follows:

    -   *VhdPath* is the path of the virtual hard disk file.
    -   *FileSize* is the new maximum size of the virtual hard disk, in bytes.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Mirror a virtual hard disk

To mirror a virtual hard disk using the [**MirrorVirtualDisk**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh448678) function, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **Storage.exe MirrorVirtualDisk** *SourcePath* **** *DestinationPath*

    where the parameters are as follows:

    -   *SourcePath* is the path of the source virtual hard disk file.
    -   *DestinationPath* is the path of the destination virtual hard disk file.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Enumerate user metadata for a virtual hard disk

To enumerate user metadata for a virtual hard disk, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **Storage.exe EnumerateUserMetaData** *VhdPath*

    where *VhdPath* is the path of the virtual hard disk file.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Update user metadata for a virtual hard disk

To update user metadata for a virtual hard disk, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **Storage.exe SetUserMetaData** *VhdPath* **** *ID*

    where the parameters are as follows:

    -   *VhdPath* is the path of the virtual hard disk file.
    -   *ID* is the new identifier for the virtual hard disk.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Obtain user metadata for a virtual hard disk

To obtain user metadata for a virtual hard disk, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **Storage.exe GetUserMetaData** *VhdPath*

    where *VhdPath* is the path of the virtual hard disk file.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Delete user metadata for a virtual hard disk

To obtain user metadata for a virtual hard disk, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **Storage.exe DeleteUserMetaData** *VhdPath*

    where *VhdPath* is the path of the virtual hard disk file.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Add a parent to a virtual hard disk

To add a parent to a virtual hard disk, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **Storage.exe AddVirtualDiskParent** *VhdPath* **** *ParentPath*

    where the parameters are as follows:

    -   *VhdPath* is the path of the virtual hard disk file.
    -   *ParentPath* is the path of the parent virtual hard disk file.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

