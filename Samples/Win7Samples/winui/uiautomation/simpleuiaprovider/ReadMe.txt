================================
UIASimpleProvider Sample
Microsoft UI Automation
================================
This sample application creates a simple custom control that supports UI Automation.

The control is a simple button-like control that supports InvokePattern. Clicking the 
button causes it to change color. You can also tab to the button and click it by pressing
the spacebar.
 
To test the functionality of InvokePattern, you can use the UISpy tool. Click on the control
in the UI Automation raw view or control view and then select Control Patterns from the
View menu. In the Control Patterns dialog box, you can call the InvokePattern::Invoke method.

===============================
Sample Language Implementations
===============================
This sample is available in the following language implementations:
     C++

=====
Files
=====
Control.cpp			Implements a simple custom control that supports UI Automation
Control.h			Declaration of the CustomButton class
Provider.cpp			Implements the Provider class (supports IRawElementProviderSimple and IInvokeProvider)
Provider.h			Declaration of the Provider class
ReadMe.txt       		This ReadMe
resource.h			VS resource file
UIASimpleProvider.cpp		Sample entry point
UIASimpleProvider.h		Includes resource.h
UIASimpleProvider.ico		Application icon
UIASimpleProvider.rc		Application resource file
UIASimpleProvider.sln		VS solution file
UIASimpleProvider.vcproj	VS project file
small.ico			Small icon
stdafx.h                        Precompiled header

==================== 
Minimum Requirements
====================
Windows XP, Windows Server 2003
Visual Studio 2008

========
Building
========
To build the sample using Visual Studio 2008:
     1. Run the Windows SDK Configuration Tool provided with the SDK to add SDK include directories to Visual Studio.
     2. Open Windows Explorer and navigate to the project directory.
     3. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     4. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To build the sample from the command line, see Building Samples in the Windows SDK release notes at the following location:
	%Program Files%\Microsoft SDKs\Windows\v7.0\ReleaseNotes.htm

=======
Running
=======
To run the sample:
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type UIASimpleProvider.exe at the command line, or double-click the icon for UIASimpleProvider.exe 
	to launch it from Windows Explorer.

To run from Visual Studio, press F5 or click Debug->Start Debugging.