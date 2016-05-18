TalkBack

Demonstrates
============
TalkBack demonstrates speech recognition (SR) and text-to-speech (TTS)
capabilities. You can speak any word or phrase and TalkBack will attempt to
recognize it, display it on the screen and even play back the actual spoken
phrase, allowing you to confirm the transcription.

Its unsophisticated interface belies the richness of the underlying tool suite.
TalkBack does everything that a major SR application needs to do and does it in
only a few lines. It initializes the SR and TTS engines, accepts input from the
microphone, and provides audio playback through speakers. It also recognizes
words, displays them on the screen, and speaks them back with the computer's TTS
voice.

Sample Language Implementations
===============================
This sample is available in C++.

Files
=====
TalkBack.cpp            Implementation of the application. 

TalkBack.rc             Resource scripts.
version.rc2

TalkBack.ico            Icon files.

TalkBack.sln            Microsoft Visual Studio solution file.

TalkBack.vcproj         Visual C++ project file.

Readme.txt              This file.

To build the sample using Visual Studio 2005 or Visual Studio 2008:
==================================================================
    1. Open Windows Explorer and navigate to the directory.
    2. Double-click the icon for the TalkBack.sln (solution) file to open the
       file in Visual Studio.
    3. In the Build menu, select Build Solution. The application will be built
       in the "Debug" or "Release" directory for 32-bit platforms, "x64\Debug"
       or "x64\Release" directory for 64-bit platforms.
    
To run the sample:
=================
    1. Navigate to the directory that contains the new executable, using the
       command prompt or Windows Explorer.
    2. Type TalkBack.exe at the command line, or double-click the icon for
       TalkBack.exe to launch it from Windows Explorer.
