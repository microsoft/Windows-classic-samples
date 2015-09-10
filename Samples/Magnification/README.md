Magnification API sample
========================

This sample demonstrates how to use the Magnification API to create a full-screen magnifier that magnifies the entire screen, and a windowed magnifier that magnifies and displays a portion of the screen in a window.

The full-screen magnifier sample (FullscreenMagnifierSample) includes an option for setting an input transform that maps the coordinate space of the magnified screen content to the actual (unmagnified) screen coordinate space. This enables the system to pass pen and touch input that is entered in magnified screen content, to the correct UI element on the screen. For the input transform option to work, you must:

-   Set `UIAccess="true"` in the application's manifest before building FullScreenMagnifierSample.
-   After building, digitally sign the FullScreenMagnifierSample.exe file.
-   Copy the signed FullScreenMagnifierSample.exe file to a secure folder before running it.

For more information, see [Security Considerations for Assistive Technologies](http://msdn.microsoft.com/en-us/library/windows/desktop/ee671610).

**Warning**  This sample requires Microsoft Visual Studio 2013 or a later version (any SKU) and will not compile in Microsoft Visual Studio Express 2013 for Windows.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related topics
--------------

[Magnification API](http://msdn.microsoft.com/en-us/library/windows/desktop/ms692162)

Related technologies
--------------------

[Magnification API](http://msdn.microsoft.com/en-us/library/windows/desktop/ms692162)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

To build this sample:

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.
2.  Go to the directory named for the sample. Go to the C++ directory and double-click the Visual Studio Solution (.sln) file.
3.  In **Solution Explorer**, right click **FullScreenMagnifierSample** and select **Properties**.
4.  In the **FullScreenMagnifierSample Property Pages** dialog box, expand the **Configuration Properties** node, and then the **Linker** node.
5.  Select **Manifest File**, and then set the manifest properties as follows.
    <table>
    <colgroup>
    <col width="50%" />
    <col width="50%" />
    </colgroup>
    <thead>
    <tr class="header">
    <th align="left">Property
    Setting</th>
    </tr>
    </thead>
    <tbody>
    <tr class="odd">
    <td align="left">Enable User Account Control (UAC)
    Yes (/MANIFESTUAC:)</td>
    <td align="left">UAC Execution Level
    highestAvailable (/level='highestAvaliable')</td>
    </tr>
    </tbody>
    </table>

6.  Click **OK**.
7.  Press F6 or use **Build** \> **Build Solution** to build the sample. Both the full-screen magnifier (FullscreenMagnifierSample) and the windowed magnifier (MagnifierSample) will be built in the default \\Debug or \\Release directory.

Run the sample
--------------

To run the full-screen magnifier sample:

1.  Navigate to the *\<install\_root\>*\\Magnification API Sample\\C++\\Debug folder for this sample using the command prompt or Windows Explorer.
2.  Use a tool such as Sign Tool (SignTool.exe) to digitally sign FullscreenMagnifierSample.exe.
3.  Copy FullscreenMagnifierSample.exe to a secure folder.
4.  In the secure folder, type FullscreenMagnifierSample.exe at the command line, or double-click the icon for FullscreenMagnifierSample.exe to launch from Windows Explorer.

To run the windowed sample:

1.  Navigate to the *\<install\_root\>*\\Magnification API Sample\\C++\\Debug folder for this sample using the command prompt or Windows Explorer.
2.  Type MagnifierSample.exe at the command line, or double-click the icon for MagnifierSample.exe to launch from Windows Explorer.

