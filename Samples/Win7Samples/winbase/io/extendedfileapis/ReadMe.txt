A new API, GetFileInformationByHandleEx, was added in the Windows Vista 
timeframe that allows a user to query the file system for extended information 
for a given file handle.  This new API has various information classes that
can be used to retrieve different types of data.

This sample demonstrates usage of several of the information classes, including
both some file specific and directory specific classes.

In addition, it demonstrates the use of another new API, OpenFileById, that
can be used to open a handle to a file when the underlying file system supports
opening the file by ID (eg. NTFS).

Files
=====

Readme.txt              This file.
ExtendedFileAPIs.cpp    Source file containing sample code.
ExtendedFileAPIs.sln    Visual Studio 2005 solution file.
ExtendedFileAPIs.vcproj Visual Studio 2005 project file.


How to Build
============

  1) Open the project in Visual Studio 2005 and build using the IDE.
  2) Open a Visual Studio command prompt and type "vcbuild" in the directory
     containing the sources.

How to Run
==========

ExtendedFileAPIs [-id] targetFile

  -id  If this flag is specified the target file is assumed to be a file ID 
       and the program will attempt to open the file using OpenFileById.  
       The current directory will be used to determine which volume to scope
       the open to.

Note that targetFile can be a file *or* a directory.
