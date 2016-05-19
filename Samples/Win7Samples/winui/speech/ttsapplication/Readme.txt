TtsApplication

Demonstrates
============
TtsApplication is an example of a text-to-speech (TTS) enabled application. This
sample application is intended to demonstrate many of the features for SAPI 5 in
a single coherent application. It is not a full featured TTS-enabled application
although the foundations of many of the options are present.

TtsApplication allows you to hear the resulting audio output from the TTS
process for text entered in the main window. Alternatively, you can open a file
and TtsApplication will speak the contents of that file. Each word is
highlighted in the text window to indicate the current TTS processing position.
 
Sample Language Implementations
===============================
This sample is available in C++.

Files
=====
TtsApplication.h        Contains the declaration of CTTSApp class.

winmain.cpp             Contains the entry point for the application. 

globals.h               Declaration of global variables.

globals.cpp             Global variables.

dlgmain.cpp             The main dialog box, and it's controls.

childwin.cpp            Implementation of child window which is used for
                        displaying the mouth bitmaps.
                        
resource.h              Microsoft Developer Studio generated include file. Used
                        by TtsApplication.rc.

TtsApplication.rc       Resource scripts.
version.rc2

TtsApplication.ico      Icon files.
res\appicon.ico

res\mic eyes closed.bmp Bitmap resources
res\mic.bmp
res\mic_eyes_narrow.bmp
res\mic_mouth_10.bmp
res\mic_mouth_11.bmp
res\mic_mouth_12.bmp
res\mic_mouth_13.bmp
res\mic_mouth_2.bmp
res\mic_mouth_3.bmp
res\mic_mouth_4.bmp
res\mic_mouth_5.bmp
res\mic_mouth_6.bmp
res\mic_mouth_7.bmp
res\mic_mouth_8.bmp
res\mic_mouth_9.bmp
res\mouthclo.bmp
res\mouthmed.bmp
res\mouthnar.bmp
res\mouthop1.bmp
res\mouthop2.bmp
res\mouthop3.bmp
res\mouthop4.bmp

TtsApplication.sln      Microsoft Visual Studio solution file.

TtsApplication.vcproj   Visual C++ project file.

Readme.txt              This file.

To build the sample using Visual Studio 2005 or Visual Studio 2008:
==================================================================
    1. Open Windows Explorer and navigate to the directory.
    2. Double-click the icon for the TtsApplication.sln (solution) file to open
       the file in Visual Studio.
    3. In the Build menu, select Build Solution. The application will be built
       in the "Debug" or "Release" directory for 32-bit platforms, "x64\Debug"
       or "x64\Release" directory for 64-bit platforms.
    
To run the sample:
=================
    1. Navigate to the directory that contains the new executable, using the
       command prompt or Windows Explorer.
    2. Type TtsApplication.exe at the command line, or double-click the icon for
       TtsApplication.exe to launch it from Windows Explorer.
