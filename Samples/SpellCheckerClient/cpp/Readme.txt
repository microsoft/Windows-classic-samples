Spell Checking Client Sample
================================
This sample demonstrates how to use the spell checking API as a client

Sample Language Implementations
===============================
C++

Files:
=============================================
SampleSpellingClient.sln
SampleSpellingClient.vcxproj
SampleSpellingClient.vcxproj.filters
commands.h
onspellcheckerchanged.h
spellprint.h
util.h
resources.rc
SampleSpellingClient.cpp

NOTE: If using VS11, this sample requires a non-Express version

To build the sample using the command prompt:
=============================================
    1. Open the Command Prompt window and navigate to the Client directory.
    2. Type msbuild SampleSpellingClient.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
    1. Open File Explorer and navigate to the Client directory.
    2. Double-click the icon for the SampleSpellingClient.sln file to open the file in Visual Studio.
    3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To install the sample:
=================
    1. Navigate to the directory that contains the new executable, using the command prompt or File Explorer.
    2. Create a folder called "SampleSpellingClient" under "Program Files".
    3. Copy SampleSpellingClient.exe to Program Files\SampleSpellingClient.

To run the sample:
=================
    Just run through the command line. The possibilities are:

    - SampleSpellingClient.exe: with no arguments, it will list all languages for which there is an available spell checker in the system
    - SampleSpellingClient.exe <language tag>: it will start an interactive spell checking session using the default provider for the language (if there's more than one, you can change it in the Control Panel)

    Session example:
    > SampleSpellingClient.exe en-us
    Language: en-us

    Provider: MsSpell (Microsoft Windows Spellchecker)
    
    Options:
    
    Commands:
    quit - Quit
    add <word> - Add word
    ac <word> <word> - Add autocorrect pair
    ign <word> - Ignore word
    chk <text> - Check text
    lst <wordlist> - List words in specified wordlist (0 = ignored words, 1 = added
    words, 2 = excluded words, 3 = autocorrect pairs)
    > chk helo
    helo [0, 3] is misspelled. Suggestions:
             hello
             halo
             helot
             help
             hell
             held
             hellos
             hero
             helm
    
    > add helo
    Spell checker changed.
    > chk helo
    No errors.
    
    > chk washington
    washington [0, 9] is misspelled. It should be autocorrected to:
            Washington
    
    > chk helo helo
    helo [5, 8] is misspelled. It should be deleted.
    
    > ign washington
    Spell checker changed.
    > chk washington
    No errors.
    
    > chk tre
    tre [0, 2] is misspelled. Suggestions:
             tree
             trek
             trey
             true
             tare
             tire
             tore
             rte.
             tee
             try
    
    > ac tre tree
    > Spell checker changed.
    chk tre
    tre [0, 2] is misspelled. It should be autocorrected to:
            tree
    
    > lst 0
    washington
    > lst 1
    helo
    > lst 2
    > lst 3
    tre     tree
    > quit