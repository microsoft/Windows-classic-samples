UI Automation document content client sample
============================================

This sample demonstrates how an application can use [Microsoft UI Automation control patterns](http://msdn.microsoft.com/en-us/library/windows/desktop/ee671194) to access the content of a document that is being displayed in another application's window.The sample is a command-line tool that can access the document content of any window that supports the [Text](http://msdn.microsoft.com/en-us/library/windows/desktop/ff384841) control pattern. The sample shows how to retrieve various types of content from the window, including headings, annotations, the current selection, and so on.

**Warning**  This sample requires Microsoft Visual Studio 2013 or a later version (any SKU) and will not compile in Microsoft Visual Studio Express 2013 for Windows.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related topics
--------------

[**IUIAutomationTextPattern**](http://msdn.microsoft.com/en-us/library/windows/desktop/ee696214)

[**IUIAutomationTextPattern2**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh437299)

[Text and TextRange Control Patterns](http://msdn.microsoft.com/en-us/library/windows/desktop/ff384841)

[UI Automation Support for Textual Content](http://msdn.microsoft.com/en-us/library/windows/desktop/ee684082)

[Working with Text-based Controls](http://msdn.microsoft.com/en-us/library/windows/desktop/ff384861)

Related technologies
--------------------

[UI Automation](http://msdn.microsoft.com/en-us/library/windows/desktop/ee684009)

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
3.  Press F7 (or F6 for Visual Studio 2013) or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

To run the sample after building it, follow these steps:

1.  Open a document in an application that supports the UI Automation Text control pattern, such as Notepad.
2.  Go to the installation folder for the UiaDocumentClient sample with Windows Explorer, and then run UiaDocumentClient.exe from the \<*install\_root*\>\\UI Automation Document Content Client Sample\\C++\\Debug folder.

    To run this sample from Microsoft Visual Studio, press the F5 key to run with debugging enabled, or Ctrl+F5 to run without debugging enabled. Alternatively, select **Start Debugging** or **Start Without Debugging** from the **Debug** menu.

3.  Within three seconds of starting the sample, place the cursor in the document window that you opened in Step 1. The sample will display a list of commands.
4.  Use the commands to retrieve various types of content from the document.

