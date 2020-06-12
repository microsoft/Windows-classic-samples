---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: IFilter sample
urlFragment: ifilter-sample
description: Demonstrates how to implement a sample IFilter for a fictitious file format .filtersample which is actually an XML file.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# IFilter sample

This C++ project implements a sample IFilter for a fictitious file format *.filtersample* which is actually an XML file.

## Files

| File | Description | 
|------|-------------|
| *FilterBase.h* | Reusable base class *CFilterBase* with generic implementation of IFilter interface.|
| *FilterSample.cpp* | Derived class *CFilterSample*, inherits from *CFilterBase*. Does actual filtering of *.filtersample* format.|
| *Dll.cpp* | Standard COM implementation of class factory and registration of *.filtersample* filter. | 
| *FilterSample.def* | Standard COM exported function list. |
| *FilterSample.vcproj* | Visual Studio project file. |
| *FilterSample.sln* | Visual Studio solution file. |
| *SampleFile.filtersample* | Sample *.filtersample* file. |
| *Readme.txt* | This readme. |


## To build and use this sample

 1. Open the *FilterSample.sln* solution file in Visual Studio.
 1. In the **Build** menu, select **Build Solution**. The output *FilterSample.dll* will be built in the default *Debug* or *Release* directory.
 1. From an elevated command prompt, run `regsvr32 FilterSample.dll` from the *Debug* or *Release* directory.
 1. Copy the *SampleFile.filtersample* file to an indexed location like the user's *Documents* folder.
 1. After giving the indexer time to index the file, it should be possible to search for terms in the document like "dog" or "broadcast".

To unregister the sample run `regsvr32 -u FilterSample.dll` from an elevated command prompt.

This sample will run on Vista and Win7.

## Additional information for testing and debugging a custom filter

These are several scenarios in which a custom filter should be tested to make sure it behaves properly.

1. In the *FiltDump.exe* test tool from the Windows SDK.
   This is the easiest way to test your filter as it loads it in a process you control.
1. In *Explorer.exe* when doing full document text searches (referred to as "grep").
1. In the search indexer, filter host process *SearchFilterHost.exe*.

### Debugging using FiltDump.exe
1. Set *FiltDump.exe* as the program to debug in the project properties debugging UI.
1. Set a break point on `::OnInit()` and then start the debugger (**F5**).
1. Repeat this with as many sample files as possible. Be sure to test corrupt or improperly formed files.

### Debugging using Explorer.exe (for the full document search case)
1. Configure the search options to always use the full document feature that will
   load your filter during the search rather than using the index. Do this 
   in the **Folder Options Search** tab:
    1. Choose **Always search file names and contents (might be slow)**.
    1. Check **Don't use the Index when searching the file system (might be slow)**.
1. Configure *explorer.exe* as the program to debug.
1. Set the command line to a folder that contains files of your type. When you start Explorer with a command line like this it creates a new instance, leaving the 
   desktop/tray Explorer process still running. An alternative is to shut down the currently running *explorer.exe* (right-click the background of the **Start** menu, choose **Exit Explorer** or use *taskman.exe* or *kill.exe* to shut down Explorer).
1. Set a break point in the `OnInit()` function or other places in your code.
1. Start the debugger, the command line will open the explorer on the folder with your files or navigate the explorer to a folder.
1. Type some text into the search box.
1. Your filter will be loaded and initialized with the files in the folder, and Explorer will display them if they match.

### Debugging using SearchFilterHost.exe

1. Since the filter host shuts itself down on a regular basis to handle incorrectly implemented filters that leak or don't correctly terminate, you need to turn off this behavior by setting this registry value.
  
   `[HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Search\Gathering Manager]
       "FilterHostProcessTimeout"=dword:00000000`
   
1. Attach the debugger to *SearchFilterHost.exe* and set some breakpoints.
1. Create, move. or modify some of your file types that are stored in an indexed location.

Since the filter might be in use, you might need to start and stop the 
search service to replace it.

     NET STOP WSEARCH
     <install your DLL>
     NET START WSEARCH
    
### Updating the index to force items to be re-indexed

The UI provides a way to rebuild the index. To do so, complete the following steps. 

1. Open the **Indexing Options** control panel.
1. Select the **Advanced** tab and select **Rebuild**.