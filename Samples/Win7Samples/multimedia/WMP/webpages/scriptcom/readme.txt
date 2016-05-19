This sample shows a simple use of script commands in a Web page.

This sample might require certain digital media files to be added to your library. To do this, run the file named loader.hta, which is located in the Windows Media Player SDK sample media folder, and follow the directions.

Click Play to start the media playing. Click Stop to stop it. 

A JScript event handler is set up that "listens" for a script command in the media. When it encounters a script command, it uses a switch statement to look for a particular value of the script command parameter. It then sets a variable that changes the color of the text through Dynamic HTML. The particular script command is also displayed dynamically in bold text.

Script commands can be placed in a media file by using Microsoft Windows Media File Editor (part of Windows Media Encoder 9 Series) or other tools.

See the Windows Media Player SDK for more information.

Copyright (c) Microsoft Corporation. All rights reserved.
