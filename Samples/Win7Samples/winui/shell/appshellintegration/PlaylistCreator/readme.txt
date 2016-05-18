Playlist Creator Sample
=======================
Demonstrates how to create a verb that operates on shell items and containers. In this case, the verb creates a playlist from the selected item or container.


Sample Language Implementations
===============================
C++


Files:
=============================================
ApplicationVerb.h
PropertyStoreReader.h
PlaylistCreator.cpp
PlaylistCreator.sln
PlaylistCreator.vcproj

COMHelpers.h
COMHelpers.cpp
PrecompiledHeaders.h
RegisterExtension.h
RegisterExtension.cpp
ShellHelpers.h
ShellHelpers.cpp
Win32Helpers.h


Command Line Arguments:
=============================================
- Embedding
You need to run the program once without this to register the verbs. Subsequently you can run it with this flag


To build the sample using the Visual Studio command prompt (make sure the latest .NET version is installed):
============================================================================================================
     1. Open the Command Prompt window and navigate to the PlaylistCreator directory.
     2. From your Visual Studio command prompt type msbuild.exe PlaylistCreator.sln.



To build the sample using Visual Studio/Visual C++ Express Edition (preferred method):
======================================================================================
     1. Open Windows Explorer and navigate to the PlaylistCreator directory.
     2. Double-click the icon for the PlaylistCreator.sln file to open the file in Visual Studio/Visual C++ Express Edition.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

Note: If you are compiling 64-bit using VC++ Express Edition, you will need to use x64 cross-compiler supplied with the Windows SDK.



To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type PlaylistCreator.exe at the command line, or double-click the icon for PlaylistCreator.exe to launch it from Windows Explorer.
     3. Right-click on any music file or folder containing music files in Explorer to create a playlist to bring up the context menu
     4. You could create a .M3U or .WPL Playlist. Playlists are created in the %userprofile%\Music\Playlists folder


Locations where the new verbs are available in the Context Menu:
================================================================
Music folder, music stacks, stacks in the Music Library and Music files