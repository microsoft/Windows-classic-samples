---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Enhanced Video Renderer sample
urlFragment: enhanced-video-renderer
extendedZipContent:
- path: LICENSE
  target: LICENSE
description: This sample implements a custom presenter for the Enhanced Video Renderer. It can be used with the DirectShow EVR filter or the Media Foundation EVR sink.
---

# Enhanced Video Renderer sample

This sample implements a custom presenter for the Enhanced Video Renderer (EVR).

The sample presenter can be used with the DirectShow EVR filter or the Media Foundation EVR sink.

This sample requires Windows Vista or later.

## To use this sample (DirectShow)

1. Build the sample.
2. Regsvr32 EvrPresenter.dll.
3. Build and run the EVRPlayer sample.
4. From the File menu, select EVR Presenter.
5. Select a file for playback.

## To use this sample (Media Foundation)

1. Build the sample.
2. Regsvr32 EvrPresenter.dll.
3. Build and run the MFPlayer2 sample.
4. From the File menu, select Open File
5. In the Open File dialog, check "Custom EVR Presenter."
6. Select a file for playback.

## Classes

The main classes in this sample are the following:

D3DPresentEngine:
    Creates the Direct3D device, allocates Direct3D surfaces for rendering, and presents the surfaces.

EVRCustomPresenter:
    Implements the custom presenter.

Scheduler:
    Schedules when a sample should be displayed.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All rights reserved.
