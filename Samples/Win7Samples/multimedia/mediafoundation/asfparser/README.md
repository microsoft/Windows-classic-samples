---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: MF_ASFParser sample
urlFragment: mf-asfparser
extendedZipContent:
- path: LICENSE
  target: LICENSE
description: This sample demonstrates how to split the stream of an ASF file by using the ASF components provided by Media Foundation.
---

# MF_ASFParser sample

- Sample: MF_ASFParser
- Sample location: \<SDK install directory>\Samples\Multimedia\MF_ASFParser
- Module: MF_ASFParser.exe
- Language: Win32
- OS Version: Windows Vista or later

File | Description
-----|-------------
MF_ASFParser.sln | Solution file
MF_ASFParser.vcproj | Project file
resource.h | Resource IDs
MF_ASFParser.rc | Resource file
Common.h | Global project header
Winmain.cpp | Entry point
ASFManager.h | CASFManager declaration. Wrapper for ASF components.
ASFManager.cpp | CASFManager class definition.
Decoder.h | CDecoder declaration. Wrapper for the decoder MFT.
Decoder.cpp | CDecoder class definition.
MediaController.h | CMediaController declaration. Handles decoded samples with GDI+ and Wavform Audio
MediaController.cpp | CMediaController class definition.
Utils.h | Debug utilities declaration.
Utils.cpp | Debug utilities definition.

## Features Demonstrated

This sample demonstrates how to split the stream of an ASF file (\*.wma/\*.wmv) by using the ASF components provided by Media Foundation. It shows how to:

- Open an ASF file.
- Enumerate the audio and video streams contained in the file.
- Select an audio or a video stream for parsing.
- Seeking within the ASF Data Object.
- Generate compressed samples for the selected stream.
- Decode audio and video samples
- Play decoded audio samples using Wavform Audio APIs that ships with the Window Multimedia SDK.
- Get bitmap data for a key frame from a decoded video sample.

## Relevant Documentation

The following topics in Media Foundation SDK documentation provides information about the procedures demonstrated in this sample:

- About ASF Files
- ASF Splitter
- ASF Content Information
- ASF Indexer
- Tutorial: Parsing an ASF Stream

## Relevant APIs

The sample demonstrates the use of the following Media Foundation APIs:

- IMFASFContentInfo Interface
- IMFASFSplitter Interface
- IMFASFIndexer Interface
- MFASFContentInfo Function
- MFCreateASFSplitter Function
- MFCreateASFIndexer Function

## Building the Sample

To build this sample:

1. From the Start->All Programs menu choose Microsoft Windows SDK -> CMD Shell
2. In the WinSDK CMD shell, navigate to Samples/Multimedia/MediaFoundationF/MF_ASFParser
3. Type vcbuild.

This creates the executable module in the Debug and the Release folder

## Usage

1. To open an ASF file. click the Open Media File... button. This displays the Open dialog box.
2. Specify the media file and click Open. Information about the media file is shown on the Information pane.
3. In the Parser Configuration, select a stream to parse.  
4. To generate samples in reverse, check Reverse.
5. To specify the start point, drag the slider to the desired location.
6. To begin parsing, click the Generate Samples button. The information about the samples are shown on the Information pane on the right.
7. To test the samples for the audio stream, click Test Audio button to play 5 secs clip.
8. To test the samples for the video stream, click Show Bitmap button. This opens another Window that displays the key frame as a bitmap.
9. To close the application, click the Close (x) button.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All rights reserved.
