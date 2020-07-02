---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: AudioClip sample
urlFragment: audioclip
extendedZipContent:
- path: LICENSE
  target: LICENSE
description: This sample demonstrates using the IMFSourceReader API to get uncompressed media data from a media file.
---

# AudioClip sample

Demonstrates using the IMFSourceReader API to get uncompressed media data from a media file.

This sample application reads audio data from a media file and writes the uncompressed audio to a WAVE file.

## Sample Language Implementations

C++

## Files

AudioClip.sln
AudioClip.vcproj
main.cpp
readme.txt

## To build the sample using the command prompt

1. Open the Command Prompt window and navigate to the AudioClip directory.
2. Type `msbuild AudioClip.sln`.

## To build the sample using Visual Studio (preferred method)

1. Open Windows Explorer and navigate to the AudioClip directory.
2. Double-click the icon for the AudioClip.sln file to open the file in Visual Studio.
3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

## To run the sample

This sample is a command-line application.

It uses the following command-line arguments:

    audioclip.exe inputfile outputfile.wav

where

    inputfile:      The name of a file that contains an audio stream.
    outputfile.wav: The name of the WAVE file to write.
