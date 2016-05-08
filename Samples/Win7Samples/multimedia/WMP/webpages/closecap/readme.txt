This sample shows a simple use of closed captioning.

This sample might require certain digital media files to be added to your library. To do this, run the file named loader.hta, which is located in the Windows Media Player SDK sample media folder, and follow the directions.

Press Play to start a video file playing. Press Stop to stop it. Captions for the spoken words will display in the blue text area below the video display area. The video file (seattle.wmv) and the text file with the captions (ccsample.smi) are in the media folder.

The video and caption display areas are created with a table. A blue table cell contains a DIV element with an ID of "CapText". The cell is used by the CaptioningID property to define where the captioning text will display. The SAMIFileName defines the location of the SAMI file. The URLs use a relative path for both files and require extra backslashes for escapement. The UI mode is given the value of "none" in a PARAM element so that the UI controls will not be displayed.

See the Windows Media Player SDK for more information on closed captioning.

Copyright (c) Microsoft Corporation. All rights reserved.
