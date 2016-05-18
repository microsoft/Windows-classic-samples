Multi-touch Gestures Application (C++)
======================================
Demonstrates how to configure and handle multi-touch gesture messages in a Win32 application.
This application will initially draw rectangle in the middle of client area. By using gestures a user can manipulate the rectangle. The available commands are:
- rectangle stretch 
    By putting two fingers on the screen and modifying distance between them by moving fingers in the opposite directions or towards each other the user can zoom in/out this rectangle.
- panning
    By touching the screen with two fingers and moving them in the same direction the user can move the rectangle. 
- rotate
    By putting one finger in the center of the rotation and then rotating the other finger around it the user can rotate the rectangle
- two finger tap
    By tapping the screen with two fingers the user can toggle drawing of the diagonals
- finger press and tap
    This gesture involves movements of two fingers. It consists first of putting one finger down. Then putting the second finger down and then lifting it up. Finally the first finger is lifted up. This gesture will change the color of the rectangle.
Demonstrates: SetGestureConfig, GetGestureInfo, CloseGestureInfoHandle, WM_GESTURENOTIFY, WM_GESTURE.

Sample Language Implementations
===============================
C++
C#

Files
=====
MTGestures.sln
MTGestures.vcproj
MTGestures.cpp
MTGestures.rc
DrawingObject.cpp
DrawingObject.h
GestureEngine.cpp
GestureEngine.h
MyGestureEngine.cpp
MyGestureEngine.h
Resource.h
readme.txt

To build the sample using the command prompt
============================================
     1. Copy sample directory outside Program Files folder.
     2. Open the Command Prompt window and navigate to the copied sample directory.
     3. Type vcbuild MTGestures.sln. The application will be built in the default \Win32 or \x64, \Debug or \Release directory.

To build the sample using Visual Studio
=======================================
     1. Copy sample directory outside Program Files folder.
     2. Open Windows Explorer and navigate to the copied sample directory.
     3. Double-click the icon for the MTGest.sln file to open the file in Visual Studio.
     4. In the Build menu, select Build Solution. The application will be built in the default \Win32 or \x64, \Debug or \Release directory.

To run the sample
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type MTGestures.exe at the command line, or double-click the icon for MTGestures.exe to launch it from Windows Explorer.     
