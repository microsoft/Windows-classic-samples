---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Windows Search Prioritization and Eventing sample
urlFragment: windows-search-prioritization-sample
description: Demonstrates how to implement and use the IRowsetEvents interface to monitor changes to items belonging to a query over a particular scope.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Windows Search Prioritization and Eventing Sample

This demo illustrates how to implement and use the IRowsetEvents interface to monitor changes to items belonging to a query over a particular scope.  This demo also shows how to use the IRowsetPrioritization interface to set the indexing speed and priority of items also belonging to that scope.

## Sample Language Implementations

C++

## Files

* *Eventing.sln*
* *Eventing.vcproj*
* *evtdemo.cpp*

## To build the sample using Visual Studio

1. Open Windows Explorer and navigate to the *ShellStorage* directory.
1. Double-click the icon for the *ShellStorage.sln* file to open the file in Visual Studio.
1. In the **Build** menu, select **Build Solution**. The application will be built in the default *Debug* or *Release* directory.

	Note that this sample requires ATL (installed with Visual Studio).

## To run the sample

1. Navigate to the directory that contains the new executable using the command prompt.
1. Type **evtdemo.exe** at the command line to see the usage.

   ```
   Allows monitoring and prioritization of indexer URLs.
	
   Eventing.exe [drive:][path] [/p[:]priority] [/t[:]duration]
	
     [drive:][path]
            Specifies drive and directory of location to watch
     /p         Prioritizes indexing at the given speed
      priority     F  Foreground      H High
                   L  Low             D Default
     /t         Specifies how long in MS to monitor query
      duration     0     Until all content is indexed
                   NNNN  Monitor for NNNN milliseconds

   Examples would be:

   Eventing.exe /p:f C:\users

   To monitor all events occurring under C:\users and to prioritize any items needed to be indexed under that location.

   Eventing.exe /p:l /t:0 C:\music

   To monitor all events occurring under C:\music and to wait until all items have been indexed before exiting.
   ```