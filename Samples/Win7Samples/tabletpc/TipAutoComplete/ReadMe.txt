TIP AutoComplete SDK sample

Demonstrates:
=============
The use of the ITipAutoCompleteProvider and ITipAutoCompleteClient APIs to allow an application to work well with the Tablet Input Panel. These APIs allow a tighther integration between the TIP and the application's autocomplete list(s).

There are two modes to the sample: Synchronous and Asynchronous (for lack of better names).

The synchronous behavior is like the TIP's behavior with the Start menu search box. The text in the TIP is always up to date with the text in the application's edit control, and there is no Insert button on the TIP. The application can show its own autocomplete list based on events to the ITipAutoCompleteProvider.

The asynchronous behavior is like the TIP's behavior with the Explorer search boxes. In this mode the user needs to press Insert on the TIP to insert the TIP's text into the edit control, but the ITipAutoCompleteProvider events that are made during handwriting in the TIP allows the application to provide its own autocomplete list that the user can select out of. The no-activation and no-focus requirements of this dropdown make this implementation slightly more complicated than just showing the autocomplete list.

The "List.txt" file contains a list of names that the application uses as its autocomplete content.

Languages:
==========
    This sample is available in the following language implementations:
    C++

Files:
======
    List.txt: names for the AutoComplete list contents
    Resource.h: resource #defines
    TIPAutoCompleteSDKSample.cpp: The interface and the code for the CTipACDialog class, which implements ITipAutoCompleteProvider and communicates with ITipAutoCompleteClient.
    TipAutoCompleteSDKSample.sln: The VC8 solution file.
    TipAutoCompleteSDKSample.vcproj: The VC8 project file.

Building the Sample:
====================
To build the sample using the command prompt:
    1. Open the Command Prompt window and navigate to the directory.
    2. Type msbuild TipAutoCompleteSDKSample.sln

To build the sample using Visual Studio 2008 (preferred method):
    1. Open Windows Explorer and navigate to the directory.
    2. Double-click the icon for the TipAutoCompleteSDKSample.sln file to open the file in Visual Studio.
    3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

Running the Sample:
===================
To run the sample:
    1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
    2. Type "TipAutoCompleteSDKSample.exe" at the command line, or double-click the icon for TipAutoCompleteSDKSample.exe to launch it from Windows Explorer.
    3. Tap with the TabletPC pen into the sample's edit box.
    4. Open the TabletPC Input Panel from the hover target or from the TIP's edge target.
    5. Handwrite some characters into the TIP.
