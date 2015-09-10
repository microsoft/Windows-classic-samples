Spell Checking Provider Sample
================================
This sample demonstrates how to create a spell checking provider conformant to the Spell Checking API.

Sample Language Implementations
===============================
C++

Files:
=============================================
SampleSpellingProvider.sln
SampleSpellingProvider.vcxproj
SampleSpellingProvider.vcxproj.filters
sampleengine.h
engineoptions.h
EnumSpellingError.cpp
EnumSpellingError.h
EnumString.h
OptionDescription.cpp
OptionDescription.h
resource.h
resources.rc
SampleSpellChecker.def
SampleSpellChecker.idl
SampleSpellCheckerModule
SampleSpellCheckProvider.cpp
SampleSpellCheckProvider.h
SampleSpellCheckProviderFactory.cpp
SampleSpellCheckProviderFactory.h
SpellingError.cpp
SpellingError.h
util.h
dummyengine.h
engineoptions.h

NOTE: This sample requires the ATL (Active Template Library) 7 or greater

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the Provider directory.
     2. Type msbuild SampleSpellingProvider.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open File Explorer and navigate to the Provider directory.
     2. Double-click the icon for the SampleSpellingProvider.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To install the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or File Explorer.
     2. Create a folder called "SampleSpellingProvider" under "Program Files".
     3. Copy SampleProvider.dll to Program Files\SampleSpellingProvider.
     4. Run (double-click) one of the reg files currentuser.reg or localmachine.reg, whether you want to install for the current user or all users, respectively.

     NOTE: If you want to install the sample in another location, be sure to modify the .reg file to use the correct path, and ensure that "ALL APPLICATION PACKAGES" has read access to this location

To run the sample:
=================
     1. Go to the Control Panel, Language Clock and Region, Language, double-click in English in the list of languages.
     2. Select "Sample Spell Checker" as the default spell checker for English.
     3. Now it will be used by Windows controls and any clients of the spell checking API (such as the sample spell checking client) as the English spell checker.
