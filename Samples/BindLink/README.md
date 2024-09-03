---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: BindLink sample
urlFragment: bindLink
description: Demonstrates how to get started with Bind Link APIs.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# BindLink Sample

This sample shows how to use the Bind Link APIs to link a local "virtualPath" to a local or remote "backingPath".
It also demonstrates how the created Bind Link can be deleted. Note that creating and removing Bind Links
requires administrator privilege.

The BindLink sample supports the following operations:
* Creating a Bind Link between the "virtualPath" and the "backingPath", optionally in read-only and/or merge modes.
* Deleting the Bind Link.

The following steps demonstrate how Bind Links work, with the help of the sample.
Follow them in order.

### Preparation
* Create a directory called C:\backingPath and create files and directories in it.
* Make sure there is no directory C:\virtualPath by looking from Explorer or command prompt.
* Open an administrative command prompt.

### Creating and removing an anchorless bind link

An anchorless bind link is one that creates a virtual path where there was no directory before.

* Continue from the "Preparation" steps above.
* Create a bind link with the command `BindLink.exe CREATE C:\virtualPath C:\backingPath`
* From Explorer or a command prompt, observe that C:\virtualPath contains the same content as C:\backingPath. 
* Run the command `BindLink.exe CREATE C:\virtualPath c:\otherBackingPath`
* The command fails with a "file not found" error because c:\otherBackingPath doesn't exist.
* Create the directory c:\otherBackingPath and create files and directories in it.
* Run the command `BindLink.exe CREATE C:\virtualPath c:\otherBackingPath`
* The command fails with an "invalid argument" because there is an existing link from C:\virtualPath.
* Remove the existing bind link with the command `BindLink.exe REMOVE C:\virtualPath`
* Observe that c:\virtualPath doesn't exist anymore.

### Creating and removing a shadow bind link

A shadow bind link is one that creates a virtual path that hides (shadows) an existing directory.

* Continue from the "Creating and removing an anchorless bind link" steps above.
* Create the directory c:\virtualPath and create files and directories in it.
* Run the command `BindLink.exe CREATE C:\virtualPath c:\otherBackingPath` - This will now succeed.
* From Explorer or a command prompt, observe that C:\virtualPath contains the same content as C:\otherBackingPath, 
  and the contents of C:\backingPath are not present in C:\virtualPath. Also, the contents of c:\virtualPath that were created before the CREATE command are not present.
* Remove the existing bind link with the command `BindLink.exe REMOVE C:\virtualPath`
* The original contents of the directory C:\virtualPath are now present.

### Creating and removing a merged bind link

A merged bind link creates a virtual path that matches an existing directory.
The contents of the existing directory are combined with the contents
of another directory.

* Continue from the "Creating and removing a shadow bind link" steps above.
* Keep the directory C:\virtualPath and its contents on the disk.
* Create a Repeat the above exercise, but this time use the /merge option when creating the binding.
     "BindLink.exe CREATE C:\virtualPath c:\otherBackingPath /merge"
* From Explorer or a command prompt or using BindLink.exe LIST, observe that C:\virtualPath contains contents from c:\otherBackingPath as well as
  C:\virtualPath before the merged link was created.
* Remove all bindlinks by doing a "BindLink.exe REMOVE C:\virtualPath". Note now that the original contents of the directory C:\virtualPath can be seen.

### Creating a read-only bind link

A read-only bind link maps the files in the backing path as read-only.

* Continue from the "Creating and removing a merged bind link" steps above.
* Keep the directory C:\virtualPath and its contents on the disk.
* Run the command `BindLink.exe CREATE C:\virtualPath c:\otherBackingPath /read-only`
* From Explorer or a command prompt, observe that C:\virtualPath contains contents only from c:\otherBackingPath.
* Try to modify the contents of one of the files from c:\otherBackingPath via the C:\virtualPath. For example, if you have a file C:\otherBackingPath\fileInOtherPath.txt, it will show in C:\virtualPath as C:\virtualPath\fileInOtherPath.txt. If you run the command `echo append to fileInOtherPath >> C:\virtualPath\fileInOtherPath.txt` the command will fail with an "access denied" error.
* The original file in C:\otherBackingPath\fileInOtherPath.txt retains its original security attributes. If you update the file via its path in otherBackingPath, the changes will be reflect in the path C:\virtualPath\fileInOtherPath.txt

### Notes

**Note**   The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server.

This sample was created for Windows 11 Version 22H2 public preview 25330.1000 using Visual Studio 2022 and using the Preview Windows SDK 10.0.25330.0.
Please provide feedback on this sample!

Build this sample to get the executable BindLink.exe.

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

## Related topics

[**Bind Link API**]()

## Operating system requirements

Windows 11 Version 22H2 Build 25330 or higher

## Build the sample

Open the solution (*.sln*) file titled *BindLink.sln* from Visual Studio.

Choose a configuration that matches your system. This will probably be x64.

Go to **Build** \> **Build Solution** to build the sample.

**Warning**  This sample requires the Windows SDK 10.0.25314.0 or higher.

## Run the sample
1. After building the sample, locate the path of the executable BindLink.exe from the Output.
2. Open an admin command prompt. 
3. Either change your current directory to the directory that contains BindLink.exe or run the executable as "PathToBindLink\BindLink.exe ...", using the command line options suggested above.

**NOTE**: When you are finished, don't forget to run BindLink.exe REMOVE to remove all bind links that you created.
