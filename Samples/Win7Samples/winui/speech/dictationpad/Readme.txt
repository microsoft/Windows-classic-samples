Dictation Pad

Demonstrates
============
Dictation Pad is an example of a speech-enabled word processor. This sample
application is intended to demonstrate many of the features for SAPI 5 in a
single coherent application. It is not a full featured speech-enabled
application, although the foundations of many of options are present.

Using Dictation Pad you can speak into a microphone and, following successful
speech recognition (SR), Dictation Pad will display the sentence on the screen
as text. The words can also be spoken back in a text-to-speech (TTS) voice,
highlighting words as they are spoken. Features include the following:

    1. Dictation - Recognizes words in any context.
    2. Command and control - Recognizes a limited selection of words and applies
       them to control the flow of Dictation Pad. This includes using speech to
       select items from menus and changing the SR mode from dictation to
       command and control. 
    3. Playback - Plays back words appearing on the screen in a TTS voice.
    4. Speakback - Keeps an audio record of the actual spoken content. You can
       play it back to confirm or verify speech recognition.
    5. Phrase tracking - Maintains a list of phrase element information. This
       can locate the parts of an SR phrase even if the dictation becomes broken
       or disjointed. It also demonstrates text replacement such as inverse text
       normalization. This is the process of converting text to numbers such as
       "one two three" into "1-2-3" or "first" into "1st."
    6. Word Alternates - Displays a list of alternates for the recognized text.
       From this list you can select a replacement for the original text.
    7. Adding words to a grammar - Demonstrates the SR engine's capability to
       add words or phrases to an existing grammar or word database. Adding a
       word allows it to be recognized on subsequent occurrences.
    8. Document management - Saves documents and opens them retaining the data
       associated with the recognized SR results.
 
Sample Language Implementations
===============================
This sample is available in C++.

Files
=====
DictationPad.h          Base definitions for DictationPad application.

DictationPad.cpp        Contains the entry point for the DictationPad
                        application and definitions of methods of CDictationPad
                        class. 

dictpad_sapi.cpp        Contains the methods of CDictationPad that pertain to
                        the SAPI interfaces used in this application.

stdafx.h                Contains the standard system include files and project
                        specific include files that are used frequently, but are
                        changed infrequently.

stdafx.cpp              Generates the precompiled header.

candidatelist.h         Contains the definition of CCandidateList class which
                        manages the recognized alternatives for dictated text.

candidatelist.cpp       Implementation details of the CCandidateList class.

dictationrun.h          Contains the definition of CDictationRun which keeps
                        track of the dictation-specific items that go with runs
                        of test that were dictated to DictationPad.

dictationrun.cpp        Implementation details of the CDictationRun class.

phrasereplace.h         Contains the definition of CPhraseReplacement class
                        which does the bookkeeping to translate between a phrase
                        as displayed with ITN replacements and a phrase as seen
                        by the associated RecoResult.

phrasereplace.cpp       Implementation details of the CPhraseReplacement class.

recomgr.h               Support for queuing insertion points and correctly
                        placing recognized text.

recomgr.cpp             Support for queuing insertion points and correctly
                        placing recognized text.

resource.h              Microsoft Developer Studio generated include file. Used
                        by DictationPad.rc.

resultcontainer.h       Contains the definition of CResultContainer class which
                        makes all of the recognition object specific SAPI 5
                        calls.

resultcontainer.cpp     Implementation details of the CResultContainer class.

textrun.h               Contains the definition of CTextRun class which is our
                        base class for holding onto all the text with
                        DictationPad.

textrun.cpp             Implementation details of the CTextRun class.

textrunlist.h           Contains the definition of CTextRunList which is a
                        container class that keeps track of all of the
                        individual text runs.

textrunlist.cpp         Implementation details of the CTextRunList class.

chs_cmdmode.xml         SAPI grammar files for various languages.
chs_dictmode.xml
cht_cmdmode.xml
cht_dictmode.xml
cmdmode.xml
deu_cmdmode.xml
deu_dictmode.xml
dictmode.xml


DictationPad.rc         Resource scripts.
chs_dictpad.rc
cht_dictpad.rc
deu_dictpad.rc
version.rc2

toolbar1.bmp            Resource for toolbar.

DictationPad.ico        Icon files.
small.ico

DictationPad.sln        Microsoft Visual Studio solution file.

DictationPad.vcproj     Visual C++ project file.

Readme.txt              This file.

To build the sample using Visual Studio 2005 or Visual Studio 2008:
==================================================================
    1. Open Windows Explorer and navigate to the directory.
    2. Double-click the icon for the DictationPad.sln (solution) file to open
       the file in Visual Studio.
    3. In the Build menu, select Build Solution. The application will be built
       in the "Debug" or "Release" directory for 32-bit platforms, "x64\Debug"
       or "x64\Release" directory for 64-bit platforms.
    
To run the sample:
=================
    1. Navigate to the directory that contains the new executable, using the
       command prompt or Windows Explorer.
    2. Type DictationPad.exe at the command line, or double-click the icon for
       DictationPad.exe to launch it from Windows Explorer.

Known issue:
===========
Running the 32-bit executable of the Dictation Pad sample on a 64-bit computer 
will cause errors. Run the 32-bit executable of the Dictation Pad sample only 
on a 32-bit computer and run the 64-bit executable of the Dictation Pad sample
only on a 64-bit computer. 
