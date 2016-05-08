Multi-touch Scratchpad Application (RTS/C#)
===========================================
Demonstrates how to enable and handle multi-touch window messages in a RealTimeStylus inking application.

MultiTouch sample application. Single window, where user can draw using multiple fingers at the same time. The trace of the primary finger, the one that was put on the digitizer first, is drawn in black. Secondary fingers are drawn in six other colors: red, green, blue, cyan, magenta and yellow. 

It is an example of the neccessary infrastructure for handling multi-touch events inside a managed application that uses Real Time Stylus API to receive multi-touch input and render the output. Demonstrates how to register a window for multi-touch inking, how to initialize RealTimeStylus and DynamicRenderer objects, how to derive and attach synchronous RTS plug to receive desired multi-touch events, through ordinary inking StylusDown/StylusUp events.It tracks finger-down count and sets the appropriate cursor color. Demonstrates: RealTimeStylus, DynamicRenderer, StylusSyncPlugin.

Sample Language Implementations
===============================
C++
C#

Files
=====
MTScratchpadRTStylus.sln
MTScratchpadRTStylus.csproj
MTScratchpadRTStylus.cs
MTScratchpadRTStylus.Designer.cs
MTScratchpadRTStylus.resx
Program.cs
AssemblyInfo.cs
Resources.Designer.cs
Resources.resx
Settings.Designer.cs
Settings.settings
readme.txt

Prerequisites
=============
This sample can be built only with Microsoft.ink.dll version 6.1 (ships with Windows 7) or newer, because it uses new MultiTouchEnabled property in Microsoft.StylusInput.RealTimeStylus interface.

To build the sample using the command prompt
============================================
     1. Copy sample directory outside Program Files folder.
     2. Open the Command Prompt window and navigate to the copied sample directory.
     3. Type msbuild MTScratchpadRTStylus.sln. The application will be built in the default bin\Debug or bin\Release directory.

To build the sample using Visual Studio
=======================================
     1. Copy sample directory outside Program Files folder.
     2. Open Windows Explorer and navigate to the copied sample directory.
     3. Double-click the icon for the MTScratchpadRTStylus.sln file to open the file in Visual Studio.
     4. In the Build menu, select Build Solution. The application will be built in the default bin\Debug or bin\Release directory.

To run the sample
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type MTScratchpadRTStylus.exe at the command line, or double-click the icon for MTScratchpadRTStylus.exe to launch it from Windows Explorer.

