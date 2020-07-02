---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: MFPlayer2 sample
urlFragment: mfplayer2
extendedZipContent:
- path: LICENSE
  target: LICENSE
description: Demonstrates how to use the IMFPMediaPlayer API to perform media playback
---

# MFPlayer2 sample

Demonstrates how to use the IMFPMediaPlayer API to perform media playback.

This sample implements the following playback features:

- Play, pause, stop
- Fast-forward and rewind
- Frame stepping
- Seeking
- Volume and mute
- Metadata
- Video zoom
- Video effects

For a more basic example of using IMFPMediaPlayer, see the SimplePlay
sample.

## Sample Language Implementations

C++

Files:

AudioSessionVolume.cpp
AudioSessionVolume.h
MainDialog.cpp
MainDialog.h
MFPlayer.h
MFPlayer.rc
MFPlayer.sln
MFPlayer.vcproj
Player2.cpp
Player2.h
readme.txt
resource.h
winmain.cpp
images\mute.bmp
images\play.bmp
images\slider.bmp
images\volume.bmp
WinUI\Slider.cpp
WinUI\Slider.h
WinUI\ThemedButton.cpp
WinUI\ThemedButton.h
WinUI\utils.h

## To build the sample using the command prompt

1. Open the Command Prompt window and navigate to the MFPlayer2 directory.
2. Type msbuild MFPlayer.sln.

## To build the sample using Visual Studio (preferred method)

1. Open Windows Explorer and navigate to the MFPlayer2 directory.
2. Double-click the icon for the MFPlayer.sln file to open the file in Visual Studio.
3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

## To run the sample

1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
2. Type MFPlayer.exe at the command line, or double-click the icon for MFPlayer.exe to launch it from Windows Explorer.
