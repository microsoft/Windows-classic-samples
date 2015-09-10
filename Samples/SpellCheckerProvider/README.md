Spell checking provider sample
==============================

This sample demonstrates how to create a sample spell checking provider that conforms to the Windows [Spell Checking API](http://msdn.microsoft.com/en-us/library/windows/desktop/hh869852). It is registered with Windows and can then be used in text controls if selected by the user in the **Language Control Panel**.

This sample works in conjunction with the [Spell checking client sample](http://go.microsoft.com/fwlink/p/?linkid=242818).

The following documentation provides general guidance for the APIs used in this sample:

-   [Spell Checking API](http://msdn.microsoft.com/en-us/library/windows/desktop/hh869852)
-   [Spell Checking API Reference](http://msdn.microsoft.com/en-us/library/windows/desktop/hh869853)

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related topics
--------------

[Spell Checking API](http://msdn.microsoft.com/en-us/library/windows/desktop/hh869852)

[Spell checking client sample](http://go.microsoft.com/fwlink/p/?linkid=242818)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.
2.  Go to the directory named for the sample, and double-click the Visual Studio Solution (.sln) file.
3.  Press F7 (or F6 for Visual Studio 2013) or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

The sample must be installed as a spell checking provider by performing the following steps.

1.  Create a folder named "SampleSpellingProvider" under "C:\\Program Files\\".
2.  Copy the SampleProvider.dll you've built to "C:\\Program Files\\SampleSpellingProvider\\".
3.  Run one of the reg files provided with this sample:
    -   currentuser.reg if you want to install for the current user.
    -   localmachine.reg if you want to install for all users.

    **Note**  The reg files contain the path "c:\\\\Program Files\\\\SampleSpellingProvider\\\\SampleSpellingProvider.dll". Edit it appropriately if you placed your .dll in a different location.
4.  Open the desktop **Control Panel**, select **Clock, Language, and Region** \> **Language**, and double-click **English** in the list of languages.
5.  Select "Sample Spell Checker" as the default spell checker for English.

The sample spell checking provider will now be used as the English spell checker by Windows controls and any clients of the spell checking API. You can install the [spell checking client sample](http://go.microsoft.com/fwlink/p/?linkid=242818) to exercise this provider sample.

