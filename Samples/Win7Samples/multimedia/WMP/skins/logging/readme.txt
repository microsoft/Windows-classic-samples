This sample provides tools for debugging and shows a simple use of logging to the hard disk.

This sample might require certain digital media files to be added to your library. To do this, run the file named loader.hta, which is located in the Windows Media Player SDK sample media folder, and follow the directions.

WARNING: The use of the tools in this folder should be limited to advanced developers of skins only and is not recommended for end users.

The tools are two simple registry files that enable or disable logging of information to hard disk. Logging is diabled by default. To enable logging, run the DebugON registry file by double clicking it. To disable logging, run the DebugOff registry file by double clicking it. You will get a dialog each time you enable or disable, asking if you want to make changes to the registry. If you click Yes, you will get a second message informing you that changes have been made.

If logging is enabled, a file will be placed on the hard disk in the same folder as the skin. The file will be named "filename_0_log.txt", where filename is the name of the skin file. The code in your skin can write text to this file using the Theme.logString attribute. This can be useful if you want to determine what is going on inside your code while it is running. Note that the text file is encoded with Unicode characters.

Also included is a simple skin called Logging.wms. This skin enables you to click on a button that sends logging information to the hard disk (if logging is enabled) and also sends a text message to a text element in the skin itself.

NOTE: Windows Media Player may send messages to the logging text file that are labeled “WARNING” that you can safely ignore. The main purpose of the logging feature is to let you store debugging information in a text file.

See the Windows Media Player SDK for more information on debugging and logging.

Copyright (c) Microsoft Corporation. All rights reserved.
