FCOPY    File copy implemented by memory mapped files.

Description
-----------
This program demonstrates how to use file mappings to implement a file-copy
program.  It specifcally shows you how to use the following Win32 APIs to
implement code that maps files into memory:

   CreateFile
   CreateFileMapping
   MapViewOfFile
   UnmapViewOfFile
   CloseHandle


Accessing Files Through File Mappings
-------------------------------------

In a nutshell, file mappings allow processes to create a region of virtual
memory that is backed by a specific file rather than the system pagefile.
The contents of the region can be paged in and out on demand just like any
other vitual memory, but will alway be paged in or out of the file being
mapped.  The process that created/opened the mapping uses pointers to this
region to access the file's data, just as it would use pointers to access a
block of dynamically-allocated memory.

File mappings can be used for two purposes:

   1) Accessing a file's contents as though it were memory
   2) Sharing memory between processes

This sample shows an application of purpose #1.

Why would you use file mappings to access files?  It may be easier to treat
a file's contents as memory (the Windows NT loader uses file mappings to load
executables, DLLs, and device drivers this way), or it may require less work
than conventional file I/O, which includes (to update an item in a file):

   Opening the file
   Allocating memory for the file's contents
   Seeking to the right place in the file
   Reading the file's contents into memory
   Modifying the memory
   Seeking back to where the file's contents should be written
   Writing it back out
   Closing the file
   Freeing the allocated memory

With file mappings, you can do the same with just:

   Open the file
   Create a mapping
   Map a view of the portion of the file needed
   Modify the contents through the view directly
   Unmap the view
   Close the mapping
   Close the file

Depending on how many separate places you have to update in the file, file
mappings can save a huge amount of seeking from place to place, and from
having to allocate and manage memory.


File Mapping Notes
------------------

When using file mappings to access a file's contents, be aware of the
following issues:

   1) If you create a mapping of a file that is larger than the file
      currently is, the file will be grown.  If the file cannot be grown, your
      request to create a mapping (i.e. call to CreateFileMapping) will fail.
      One reason that a file cannot be grown is if the file is read-only;
      another is that on Windows NT, you may not have permission to write to
      the file.

   2) Once a file mapping is created, it cannot be resized.  You have to
      close the mapping and create a new one.

   3) You can close a mapping before unmapping the views, or vice-versa.
      As long as there are open views, the mapping object will not be
      destroyed even if the mapping handle is closed.  This is because
      each view of a mapping refers to the mapping object, thereby keeping
      the object's reference count above zero.

   4) The corrollary to #3 is that in order to clean up a file mapping,
      you must unmap all of the views and close all handles to the file
      mapping object.  Failure to do so is a leak.


How to Build  FCOPY
-------------------

Debug build:
   nmake

Release build:
   nmake "nodebug =1"


How to Execute FCOPY
--------------------

FCOPY runs on Windows NT and Windows 95 and Windows 98.

Run FCOPY from the command line and specify a source filename followed by a
destination filename.  For example:

   c:>fcopy important.txt  backup.txt

