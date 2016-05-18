Sample Text-to-Speech Engine and MakeVoice

Demonstrates
============
The sample text-to-speech (TTS) engine demonstrates the design, compilation,
installation, and testing for engines. The sample consists of two projects:
SampleTtsEngine and MakeVoice.

SampleTtsEngine is the sample engine dll. This is a useful example code for
understanding how a real TTS engine might be implemented.

MakeVoice creates text-to-speech (TTS) voice fonts for sample use. It combines a
series of individually spoken words into a single file. The resulting file is
automatically loaded and you may use it in any TTS application recognizing SAPI
5 voices. MakeVoice is intended to demonstrate making sample voice fonts. The
limited scope is not ideal to create larger, more robust voice samples.
Additionally, extensive error checking or error prevention routines are not
included so that the resulting file may contain conditions not optimal to
superior performance.
 
Sample Language Implementations
===============================
This sample is available in C++.

Files
=====
SampleTtsEngine\SampleTtsEngine.cpp     Implementation of DLL Exports.

SampleTtsEngine\SampleTtsEngine.idl     This file will be processed by the MIDL
                                        tool to produce the type library and
                                        marshalling code.
                        
SampleTtsEngine\stdafx.h                Contains the standard system include
                                        files and project specific include files
                                        that are used frequently, but are
                                        changed infrequently.

SampleTtsEngine\stdafx.cpp              Generates the precompiled header.
                        
SampleTtsEngine\ttsengobj.h             Contains the declaration of the
                                        CTTSEngObj class, which is the main
                                        text-to-speech object                     
                        
SampleTtsEngine\ttsengobj.cpp           Implementation of the CTTSEngObj class.

SampleTtsEngine\ttsengver.h             Version information.

SampleTtsEngine\SampleTtsEngine.def     Export definition file.

SampleTtsEngine\ttsengobj.rgs           Registration script.

SampleTtsEngine\resource.h              Microsoft Developer Studio generated
                                        include file. Used by SampleTtsEngine.rc

SampleTtsEngine\SampleTtsEngine.rc      Resource scripts.
SampleTtsEngine\version.rc2

SampleTtsEngine\SampleTtsEngine.vcproj  Visual C++ project file.

MakeVoice\MakeVoice.cpp                 Main function for MakeVoice.

MakeVoice\stdafx.h                      Contains the standard system include
                                        files and project specific include files
                                        that are used frequently, but are
                                        changed infrequently.

MakeVoice\stdafx.cpp                    Generates the precompiled header.

MakeVoice\MakeVoice.rc                  Resource scripts.
MakeVoice\version.rc2

MakeVoice\wordlist.txt                  The list of words that are supported by
                                        the sound font. See MSDN documentation
                                        about SAPI samples and MakeVoice for
                                        more information.

MakeVoice\anglais.wav                   Wave audio files for the words that are
MakeVoice\as.wav                        supported by the sound font.
MakeVoice\blah.wav
MakeVoice\computer_s.wav
MakeVoice\default.wav
MakeVoice\enter.wav
MakeVoice\have.wav
MakeVoice\he.wav
MakeVoice\here.wav
MakeVoice\je.wav
MakeVoice\parler.wav
MakeVoice\peut.wav
MakeVoice\said.wav
MakeVoice\sample.wav
MakeVoice\selected.wav
MakeVoice\she.wav
MakeVoice\speak.wav
MakeVoice\spoken.wav
MakeVoice\text.wav
MakeVoice\the.wav
MakeVoice\to.wav
MakeVoice\tts.wav
MakeVoice\type.wav
MakeVoice\voice.wav
MakeVoice\wish.wav
MakeVoice\you.wav

MakeVoice\MakeVoice.vcproj              Visual C++ project file.

SampleTtsEngine.sln                     Microsoft Visual Studio solution file.

Readme.txt                              This file.

To build the sample using Visual Studio 2005 or Visual Studio 2008:
==================================================================
    1. Open Windows Explorer and navigate to the directory.
    2. On Windows XP, double-click the icon for the SampleTtsEngine.sln
       (solution) file to open the file in Visual Studio. On Windows 7 run
       the Visual Studio 2005 (or VS 2008) as administrator by right clicking
       the Visual Studio icon and selecting "Run as administrator". Then open
       the solution file from the "File -> Open -> Project/Solution" menu.
    3. In the Build menu, select Build Solution. The sample engine will be built
       in the "Debug" or "Release" directory for 32-bit platforms, "x64\Debug"
       or "x64\Release" directory for 64-bit platforms. The dll will
       automatically register itself in the Post-Build event of SampleTtsEngine
       project. The sound font will be installed automatically in the Post-Build
       event of MakeVoice project.
    
To run the sample:
=================
There are various ways to see the sample engine running:
    1. Use the TtsApplication sample which allows you to experiment tts voices.
       Select the "Sample TTS Voice" to use this sample. Or,
    2. Programatically create and use this sample voice. Or,
    3. On Windows 7, go to the "Control Panel -> Ease of Access -> Speech
       Recognition Options -> Text to Speech" and select the "Sample TTS Voice"
       in the voice section. All speech applications will now use this voice.
       As a confirmation of the selection, you will hear the voice speak, "You
       have selected the Sample TTS voice as the computer's default voice." 