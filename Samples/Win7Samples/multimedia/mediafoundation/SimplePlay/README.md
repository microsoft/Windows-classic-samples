---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: SimplePlay sample
urlFragment: simplePlay-sample
description: Demonstrates audio/video playback using the IMFPMediaPlayer API.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# SimplePlay sample

Demonstrates audio/video playback using the IMFPMediaPlayer API.

## Sample language implementations

C++

## Files

- *README.md*
- *resource.h*
- *SimplePlay.rc*
- *SimplePlay.sln*
- *SimplePlay.vcproj*
- *winmain.cpp*

## Build the sample using the command prompt

1. Open the Command Prompt window and navigate to the *SimplePlay* directory.
1. Type **msbuild SimplePlay.sln**.


Build the sample using Visual Studio (preferred method)

1. Open Windows Explorer and navigate to the *SimplePlay* directory.
1. Double-click the icon for the *SimplePlay.sln* file to open the file in Visual Studio.
1. In the **Build** menu, select **Build Solution**. The application will be built in the default *\Debug* or *\Release* directory.


##  Run the sample

1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
1. Type **SimplePlay.exe** at the command line, or double-click the icon for *SimplePlay.exe* to launch it from Windows Explorer.
     

To play a file, select **Open File** from the **File** menu.

To pause, press the **Spacebar**. To resume playback, press the **Spacebar** again.