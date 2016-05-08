SimpleAudio - Custom Audio Object

Demonstrates
============
This sample is intended to help developers write custom audio objects.
Application developers can use this tool to direct speech data from memory into
SAPI for speech recognition (SR) and for text-to-speech (TTS). The object does
not generate or consume any audio data. Instead, it works as an audio buffer
manager. For SR, audio data is passed to this object using a custom method
ISpAudioPlug::SetData. SAPI retrieves the audio data from this object using
IStream::Read. For TTS, audio data is passed from SAPI to this object using
Istream::Write and the audio data can be retrieved calling a custom method
ISpAudioPlug::GetData.

Sample Language Implementations
===============================
This sample is available in C++.

Files
=====
SimpleAudio.cpp         Implementation of DLL Exports.

SimpleAudio.idl         This file will be processed by the MIDL tool to produce
                        the type library and marshalling code.
                        
stdafx.h                Contains the standard system include files and project
                        specific include files that are used frequently, but are
                        changed infrequently.

stdafx.cpp              Generates the precompiled header.

SpAudioPlug.h           Contains the declaration of the SpAudioPlug class.

SpAudioPlug.cpp         Implementation of SpAudioPlug and DLL registration.

SimpleAudio.def         Export definition file.

resource.h              Microsoft Developer Studio generated include file. Used
                        by SimpleAudio.rc.

SimpleAudio.rc          Resource scripts.
version.rc2

SpAudioPlug.rgs         Registration script.

SimpleAudio.sln         Microsoft Visual Studio solution file.

SimpleAudio.vcproj      Visual C++ project file.

Readme.txt              This file.

To build the sample using Visual Studio 2005 or Visual Studio 2008:
==================================================================
    1. Open Windows Explorer and navigate to the directory.
    2. On Windows 7 run Visual Studio 2005 (or VS 2008) as administrator by
       right clicking the Visual Studio icon andselecting "Run as 
       administrator". Then open the solution file from the 
       "File -> Open ->Project/Solution" menu.
    3. In the Build menu, select Build Solution. The sample engine will be built
       in the "Debug" or "Release" directory for 32-bit platforms, "x64\Debug"
       or "x64\Release" directory for 64-bit platforms. It will automatically
       register itself in the Post-Build event.

Note:
====
The simpleAudio sample utlizes the Microsoft Speech API (SAPI) version 5.3 
available on Windows Vista or later operating system versions. 


