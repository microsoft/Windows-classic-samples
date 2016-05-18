Reco

Demonstrates
============
Reco is a speech tool that you can use to examine and test the speech process.
You may speak using a microphone for either dictation or command and control.
Alternatively, you may directly type a command and control order in an edit box
and submit it to the speech recognition engine.

During the recognition process, events will be displayed in the top window as
they occur. An event log is kept, and to examine the details of each one 
just click the event. You may use this log to trace the sequence and number of
events.

Sample Language Implementations
===============================
This sample is available in C++.

Files
=====
reco.h                  Declares the various dialog classes.

reco.cpp                Contains the entry point for the Reco application and
                        definitions of methods of dialog classes. 

stdafx.h                Contains the standard system include files and project
                        specific include files that are used frequently, but are
                        changed infrequently.

stdafx.cpp              Generates the precompiled header.
                        
lmadapt.cpp             Language Model adaptation routines.

resource.h              Microsoft Developer Studio generated include file. Used
                        by reco.rc.

chs_sol.xml             SAPI grammar files for various languages.
cht_sol.xml
deu_cardinals.xml
eng_sol.xml
esp_dates.xml
fra_cardinals.xml
itn_j.xml
jpn_sol.xml
kor_cardinals.xml
sol.ENG.xml
sol.xml

reco.rc                 Resource scripts.
chs_reco.rc
cht_reco.rc
deu_reco.rc
eng_reco.rc
esp_reco.rc
fra_reco.rc
jpn_reco.rc
kor_reco.rc
version.rc2

reco.ico                Icon files.
small.ico

Reco.sln                Microsoft Visual Studio solution file.

Reco.vcproj             Visual C++ project file.

Readme.txt              This file.

To build the sample using Visual Studio 2005 or Visual Studio 2008:
==================================================================
    1. Open Windows Explorer and navigate to the directory.
    2. Double-click the icon for the Reco.sln (solution) file to open the file
       in Visual Studio.
    3. In the Build menu, select Build Solution. The application will be built
       in the "Debug" or "Release" directory for 32-bit platforms, "x64\Debug"
       or "x64\Release" directory for 64-bit platforms.
    
To run the sample:
=================
    1. Navigate to the directory that contains the new executable, using the
       command prompt or Windows Explorer.
    2. Type Reco.exe at the command line, or double-click the icon for Reco.exe
       to launch it from Windows Explorer.

Note:
====
The Reco sample utilizes the Microsoft Speech API (SAPI) version 5.3 available 
on Windows Vista or later operating system versions. 

Known issues:
============
Some Windows Speech Recognition commands, such as "show numbers", cannot be 
triggered when executing the Reco sample. 

