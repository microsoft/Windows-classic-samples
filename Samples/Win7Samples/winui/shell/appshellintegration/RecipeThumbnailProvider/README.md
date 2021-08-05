---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Explorer thumbnail provider
urlFragment: recipethumbnailprovider
description: Illustrates how to create a Thumbnail provider that extends the explorer, providing thumbnails for a particular file type.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Explorer thumbnail provider

Illustrates how to create a Thumbnail provider that extends the explorer, providing thumbnails for a particular file type.

**Note** that Explorer may choose not to show thumbnails even if a provider is available.
For example, a file that has been archived to tape will not be recalled to obtain its thumbnail.

Debugging thumbnail handlers is difficult for several reasons.

1) The Windows Explorer hosts thumbnail providers in an isolated process to get robustness and improve security. Because of this it is difficult to debug your handler as you cannot set breakpoints on your code in the explorer.exe process as it is not loaded there. The isolated process is DllHost.exe and this is used for other purposes so finding the right instance of this process is difficult. 

2) Once a thumbnail is computed for a particular file it is cached and your handler won’t be called again for that item unless you invalidate the cache by updating the modification date of the file. Note that this cache works even if the files are renamed or moved.

Given all of these issues, the easiest way to debug your code is in a test application,
such as the [UsingThumbnailProviders](https://github.com/microsoft/Windows-classic-samples/tree/master/Samples/Win7Samples/winui/shell/appplatform/UsingThumbnailProviders) sample.
Once you have proven it works there, test it in the context of Explorer.

This app will take a command line parameter that names the file to get the thumbnail from and a second param that names the size to request. 

    ThumbnailProvider.exe <path to your file type> <image size>

Another step you can take to improve the debugging experience
is to disable the process isolation feature of Explorer.
You can do this by putting the following named value on the CLSID of your handler:

    HKCR\CLSID\{CLSID of Your Handler}
        DisableProcessIsolation=REG_DWORD:1

*** Be sure to not ship your handler with this on as customers require the security and robustness benefits of the isolated process feature ***
