CIMFS API Sample Application
============================

This sample demonstrates how to use the CIMFS APIs to create, configure, and manipulate CIMFS Images. The sample demonstrates how to perform each of the following operations:

-	Create a new CIM image using the **CimCreateImage** method.
-   Commit changes to the image using the **CimCommitImage** method.
-   Add a file from the local filesystem to the image using the **CimCreateFile** and **CimWriteStream** methods.
-   Mount and validate image contents using the **CimMountImage** method.
-   Add a hardlink to an existing file in the image using the **CimCreateHardLink** method.
-   Fork from the base image.
-	Delete a file from the forked CIM image.

Operating system requirements
-----------------------------

Client

Windows 10 19H1 and above

Server

Windows Server 2019 and above

Build the sample
----------------

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file titled Networking.sln.

3.  Press F7 (or F6 for Microsoft Visual Studio 2013) or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

**CimFSAPI.exe** [cim_path] [image_name] [file_to_add_path] [image_relative_path]

cim_path - The path to the location where the generated cims will be stored. e.g. C:\cim
image_name - The name of the cim image. e.g. test.cim
file_to_add_path - The path on the source filesystem of a file to be added to the cim. e.g. C:\Windows\System32\ntdll.dll
image_relative_path - The path in the cim to the source file. e.g. dir\ntdll.dll

This will create 2 cim images:

The first CIM will contain the file specified by file_to_add_path at the image_relative_path and a hardlink to that file in the same directory.
The second CIM will be a fork of the first CIM, with the original file deleted but the hardlink still present.
