---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Decoder sample
urlFragment: decoder
extendedZipContent:
- path: LICENSE
  target: LICENSE
description: This sample demonstrates how to implement a decoder in Media Foundation.
---

# Decoder sample

Demonstrates how to implement a decoder in Media Foundation.

This sample implements a "fake" MPEG-1 decoder. The decoder does not actually decode the video streqam. Instead, it simply scans the bitstream for start codes. Then it finds the time code for each payload and outputs a blank video frame with the time code.

NOTE: An earlier version of this sample was built into the MPEG1Source sample.

## Sample Language Implementations

C++

## To build the sample using the command prompt

1. Open the Command Prompt window and navigate to the Transcode directory.
2. Type msbuild Decoder.sln.

## To build the sample using Visual Studio (preferred method)

1. Open Windows Explorer and navigate to the Decoder directory.
2. Double-click the icon for the Decoder.sln file to open the file in Visual Studio.
3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

## To run the sample

This sample builds a DLL that must be registered.

To register the DLL:

1. Open an elevated command prompt.
2. Naviage to the Debug or Release directory.
3. Type "regsvr32 decoder.dll".

To unegister the DLL, type "regsvr32 /u decoder.dll"

You can use the decoder DLL with the MPEG1Source sample to play an 
MPEG-1 video file.
