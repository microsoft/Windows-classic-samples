---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: MFT_AudioDelay sample
urlFragment: mft-audiodelay
extendedZipContent:
- path: LICENSE
  target: LICENSE
description: This sample implements an audio delay effect as a Media Foundation transform (MFT) and demonstrates how to write an audio MFT.
---

# MFT_AudioDelay sample

Implements an audio delay effect as a Media Foundation transform (MFT).

Demonstrates how to write an audio MFT.

This MFT is a 1-input, 1-output transform with fixed streams. It accepts any PCM audio type. The input and output formats must be identical.

The MFT maintains a circular buffer of the last N audio samples that were received as input. To produce the output data, each input sample is mixed with the next sample on the circular buffer. The percentage of each sample in the output is the "wet/dry" mix:

`output_sample = (1.0 - wet) * input_sample + wet * delay_sample`

where "wet" ranges [0..1]

The length of the buffer determines the length of the delay. If the client drains the MFT, the MFT produces an "effect tail," which is the trailing end of the delay effect.

The application can set the delay length and the wet/dry mix using the following attributes:

- MF_AUDIODELAY_WET_DRY_MIX
- MF_AUDIODELAY_DELAY_LENGTH

This sample requires Windows Vista or later.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All rights reserved.
