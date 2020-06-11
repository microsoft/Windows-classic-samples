---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: WavSource sample
urlFragment: wavSource-sample
description: Demonstrates how to write a custom media source for Media Foundation.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# WavSource sample

This sample implements a media source that parses audio *.wav* files. The sample demonstrates how to write a custom media source for Media Foundation. It also shows how to register a byte-stream handler for the source, so that the Media Foundation source resolver can discover the media source.

To keep the sample code as simple as possible, the media source only parses *.wav* files that contain uncompressed PCM audio, mono or stereo. 

This sample does not support protected playback.

This sample requires Windows Vista or later.


## Windows 7

In Windows 7, Media Foundation already provides an in-box byte-stream handler for .wav files, and the registry entry for it is protected using Windows Resource Protection (WRP). Therefore, the sample DLL will fail to register for .wav files.

Instead, the sample registers the byte-stream handler for the file extension ".xyz". To run the sample, rename a .wav to use this file extension.

## Usage

The sample builds a DLL. To use the sample, do the following.

## Windows Vista

1. Register the DLL with *Regsvr32.exe*.

2. Open a *.wav* file from a Media Foundation playback application.

## Windows 7

1. Register the DLL with *Regsvr32.exe*.

2. Take an existing *.wav* file and rename the file extension to *.xyz*.

2. Open the *.xyz* file from a Media Foundation playback application.
   
   Note: You must run as administrator to register the DLL.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All rights reserved.