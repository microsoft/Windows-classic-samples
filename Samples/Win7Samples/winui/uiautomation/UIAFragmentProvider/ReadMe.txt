================================
UIAFragmentProvider Sample
Microsoft UI Automation
================================
This sample shows how to implement a UI Automation provider for the elements of a fragment 
that is hosted in a window (HWND) object. The control itself has been kept simple. It does not
support scrolling and therefore an arbitrary limit has been set on the number of items it
can contain. For convenience, list items are stored in a deque (from the Standard Template Library).
 
The fragment consists of the root element (a list box) and its children (the list items.)
The UI Automation provider for the list box supports only one control pattern, the Selection 
pattern. The provider for the list items implements the SelectionItem pattern.

To see the UI Automation provider at work, run the application and then open UI Spy. You can
see the application and its controls in the Control View. Clicking on the control or on the
list items in the UI Spy tree causes the provider's methods to be called, and the values 
returned are displayed in the Properties pane. (Note that some values are retrieved from the
default HWND provider, not this custom implementation.) You can also select an item in the
list by using the SelectionItem control pattern in UI Spy.

===============================
Sample Language Implementations
===============================
This sample is available in the following language implementations:
     C++

=====
Files
=====
CustomControl.cpp			Implementation of the custom list control
CustomControl.h				Declarations for the custom list control
UIAFragmentProvider.cpp			Main application entry point
UIAFragmentProvider.ico			Application icon
UIAFragmentProvider.rc			Application resource file
UIAFragmentProvider.sln			VS solution file
UIAFragmentProvider.vcproj		VS project file
ListItemProvider.cpp			Implementation of the ListItemProvider class
ListProvider.cpp			Implementation of the ListProvider class
ReadMe.txt       			This ReadMe
resource.h				VS resource file
small.ico				Small icon
stdafx.h                                Precompiled header
UIAProviders.h				Declarations for sample UI Automation provider implementations

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
     2. Type UIAFragmentProvider.exe at the command line, or double-click the icon for UIAFragmentProvider.exe 
	to launch it from Windows Explorer.

To run from Visual Studio, press F5 or click Debug->Start Debugging.