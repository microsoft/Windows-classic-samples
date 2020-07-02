---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Grayscale transform sample
urlFragment: grayscale-transform
extendedZipContent:
- path: LICENSE
  target: LICENSE
description: This sample demonstrates how to write a Media Foundation transform that implements a simple grayscale video effect.
---

# Grayscale transform sample

This sample demonstrates how to write a Media Foundation transform that implements a video effect.

This transform implements a simple grayscale video effect. It supports several YUV formats (YUY2, UYVY, NV12).

Usage:

The sample builds a DLL. To use the sample, do the following:

On Windows 7:

1. Register the DLL with Regsvr32.exe.

   Note: You must run as administrator to register the DLL.

2. Build and run the MFPlayer2 sample.

3. From the "Options" menu, select "Video Effect"

4. Open a video file.

On Windows Vista:

1. Register the DLL with Regsvr32.exe.

   Note: You must run as administrator to register the DLL.

2. Build and run the PlaybackFX sample.

3. Open a .wmv file.

This sample requires Windows Vista or later.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All rights reserved.
