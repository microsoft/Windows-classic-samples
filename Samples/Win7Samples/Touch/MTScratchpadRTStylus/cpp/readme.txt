Multi-touch Scratchpad Application (RTS/C++)
============================================
Demonstrates how to enable and handle multi-touch window messages in a RealTimeStylus inking application.

MultiTouch sample application. Single window, where user can draw using multiple fingers at the same time. The trace of the primary finger, the one that was put on the digitizer first, is drawn in black. Secondary fingers are drawn in six other colors: red, green, blue, cyan, magenta and yellow. 

It is an example of the neccessary infrastructure for handling multi-touch events inside a Win32 application that uses Real Time Stylus API to receive multi-touch input and render the output. Demonstrates how to register a window for multi-touch inking, how to initialize IRealTimeStylus and IDynamicRenderer objects, how to derive and attach synchronous RTS plug to receive desired multi-touch events, through ordinary inking events: StylusDown, StylusUp and Packets. It tracks finger-down count and sets the appropriate cursor color. Demonstrates: IRealTimeStylus3, IRealTimeStylus, IDynamicRenderer, IStylusSyncPlugin.

Sample Language Implementations
===============================
C++
C#

Files
=====
MTScratchpadRTStylus.sln
MTScratchpadRTStylus.vcproj
MTScratchpadRTStylus.cpp
MTScratchpadRTStylus.rc
Resource.h
readme.txt

To build the sample using the command prompt
============================================
     1. Copy sample directory outside Program Files folder.
     2. Open the Command Prompt window and navigate to the copied sample directory.
     3. Type vcbuild MTScratchpadRTStylus.sln. The application will be built in the default \Win32 or \x64, \Debug or \Release directory.

To build the sample using Visual Studio
=======================================
     1. Copy sample directory outside Program Files folder.
     2. Open Windows Explorer and navigate to the copied sample directory.
     3. Double-click the icon for the MTScratchpadRTStylus.sln file to open the file in Visual Studio.
     4. In the Build menu, select Build Solution. The application will be built in the default \Win32 or \x64, \Debug or \Release directory.

To run the sample
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type MTScratchpadRTStylus.exe at the command line, or double-click the icon for MTScratchpadRTStylus.exe to launch it from Windows Explorer.     
