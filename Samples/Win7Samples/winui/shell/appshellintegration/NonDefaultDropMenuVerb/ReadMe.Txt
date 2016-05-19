NonDefaultDropMenuVerb

The NonDefaultDropMenuVerb sample demonstrates how to extend the drag-and-drop context menu.

Sample Language Implementations
===============================
     This sample is available in the following language implementations:
     C++ 

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type msbuild NonDefaultDropMenuVerb.sln.


To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the NonDefaultDropMenuVerb.sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
=================

     1. Copy NonDefaultDropMenuVerb.dll to the system directory (for example, c:\windows\system).

        -or-

        Modify NonDefaultDropMenuVerb.reg file to points to a fully qualified path to the DLL.

     2. Run the command: regedit NonDefaultDropMenuVerb.reg.

     3. Use the right mouse button to drag a file from one folder to another. You will see additional menu items for the drop operation.
