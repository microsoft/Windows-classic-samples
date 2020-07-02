---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: MPEG-1 source sample
urlFragment: mpeg-1-source
extendedZipContent:
- path: LICENSE
  target: LICENSE
description: This sample contains an MPEG-1 source for Media Foundation.
---

# MPEG-1 source sample

This sample contains an MPEG-1 source for Media Foundation.

- The source parses MPEG-1 systems-layer streams and generates samples that contain MPEG-1 payloads.
- It does not support files that contain a raw MPEG-1 video or audio stream.
- It does not support seeking.

This sample requires Windows Vista or later.

## To use this sample

1. Build the sample.
2. Regsvr32 MPEG1Source.dll
3. Use the BasicPlayback sample to play an MPEG-1 video file.  

## Classes

Buffer: Resizable buffer used to hold the MPEG-1 data.

MPEG1ByteStreamHandler: Bytestream handler for the MPEG-1 source.

MPEG1Source: MPEG-1 source. Implements IMFMediaSource.

MPEG1Stream: MPEG-1 stream. Implements IMFMediaStream.

Parser: MPEG-1 elementary stream parser.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All rights reserved.
