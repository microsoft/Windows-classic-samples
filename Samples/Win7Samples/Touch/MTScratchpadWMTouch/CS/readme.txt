Multi-touch Scratchpad Application (WM_TOUCH/C#)
================================================
Demonstrates how to enable and handle multi-touch window messages in a managed application.

MultiTouch sample application. Single window, where user can draw using multiple fingers at the same time. The trace of the primary finger, the one that was put on the digitizer first, is drawn in black. Secondary fingers are drawn in six other colors: red, green, blue, cyan, magenta and yellow. 

It is an example of the neccessary infrastructure for handling multi-touch events inside a managed application. The application receives WM_TOUCH window messages. Demonstrates how to register a window to receive WM_TOUCH messages; how to unpack WM_TOUCH message parameters and how to use them. In addition, it also shows how to store the strokes and draw them on appropriate events. Demonstrates: RegisterTouchWindow, GetTouchInputInfo, CloseTouchInputHandle, WM_TOUCH.

Sample Language Implementations
===============================
C++
C#

Files
=====
MTScratchpadWMTouch.sln
MTScratchpadWMTouch.csproj
MTScratchpadWMTouch.cs
MTScratchpadWMTouch.Designer.cs
MTScratchpadWMTouch.resx
Program.cs
Stroke.cs
WMTouchForm.cs
AssemblyInfo.cs
Resources.Designer.cs
Resources.resx
Settings.Designer.cs
Settings.settings
readme.txt

To build the sample using the command prompt
============================================
     1. Copy sample directory outside Program Files folder.
     2. Open the Command Prompt window and navigate to the copied sample directory.
     3. Type msbuild MTScratchpadWMTouch.sln. The application will be built in the default bin\Debug or bin\Release directory.

To build the sample using Visual Studio
=======================================
     1. Copy sample directory outside Program Files folder.
     2. Open Windows Explorer and navigate to the copied sample directory.
     3. Double-click the icon for the MTScratchpadWMTouch.sln file to open the file in Visual Studio.
     4. In the Build menu, select Build Solution. The application will be built in the default bin\Debug or bin\Release directory.

To run the sample
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type MTScratchpadWMTouch.exe at the command line, or double-click the icon for MTScratchpadWMTouch.exe to launch it from Windows Explorer.     
