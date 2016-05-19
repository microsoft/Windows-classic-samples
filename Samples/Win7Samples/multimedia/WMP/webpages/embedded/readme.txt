This sample shows how to embed the Player in a web page and provides standard error-handling techniques.

This sample might require certain digital media files to be added to your library. To do this, run the file named loader.hta, which is located in the Windows Media Player SDK sample media folder, and follow the directions.

Buttons are provided to perform the following tasks:

* Play a known audio file.
* Play a known video file.
* Pause/Resume the media that is playing.
* Stop the media that is playing.
* Show how an error occurs and is handled.

The Player is embedded in the web page with the id of "Player" and given a default size of 200x200 pixels. The UI mode is set to "none" at runtime so that no controls are displayed, but the video window is still present. The autoStart value is true so that the Player will start playing when a button is pressed.

A separate error handler script is provided for error handling. It will be called when the Player raises an error event. The error descriptions for all errors are built into a string and displayed in a message box. The error queue is cleared when all errors have been displayed.

A separate status handler script is provided that processes the PlayStateChange event. Play states are displayed. If the Player is playing, the name of the file is displayed as well. An event handler is provided for the StatusChange event as well, and status is displayed.

In addition to processing Player errors, an error handler is provided for catching simple JScript errors.

The pause and stop buttons are disabled when not applicable.

For more information on embedding, see the Windows Media Player SDK.

Copyright (c) Microsoft Corporation. All rights reserved.
