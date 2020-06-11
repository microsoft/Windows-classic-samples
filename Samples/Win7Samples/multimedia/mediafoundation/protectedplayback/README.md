---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: ProtectedPlayback sample
urlFragment: protectedPlayback-sample
description: Demonstrates how to play DRM-protected media files in Media Foundation.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# ProtectedPlayback sample

This sample demonstrates how to play DRM-protected media files in Media Foundation. 

Much of the code in this sample is identical to the BasicPlayback sample. You may want to look at the BasicPlayback sample first, and then examine the differences between that sample and the ProtectedPlayback sample. 

The ProtectedPlayback sample has the following differences:

- *winmain.cpp*: Includes code to handle content enabler events. See the OnContentEnablerMessage function.

- *player.cpp*: The code to create the media session is different. See the CPlayer::CreateSession method. 

- *ContentEnabler.h*, *ContentEnabler.cpp*: Implements the IMFContentProtectionManager interface.
 
- *WebHHelper.h*, *WebHelper.cpp*: Manages the browser control, for non-silent enabler actions.

This sample requires Windows Vista or later.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All rights reserved.