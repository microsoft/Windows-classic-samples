A new API, CancelSynchronousIo, was added in the Windows Vista timeframe that
allows a user to attempt to cancel an outstanding synchronous operation on
a targetted thread.

This sample demonstrates safe usage of this API to cancel a call to
CreateFile that is made 'asynchronous' by posting it to another thread.


Files
=====

README.TXT              This file.
IoCancellation.h        Header file for Io Cancellation routines.
IoCancellation.c        Source file for Io Cancellation routines.
AsyncCreateFile.c       Source file for usage of cancellation for CreateFile.
AsyncCreate.sln         Visual Studio 2008 solution file.
AsyncCreate.vcproj      Visual Studio 2008 project file.


How to Build
============

  1) Open the project in Visual Studio 2008 and build using the IDE.
  2) Open a Visual Studio command prompt and type "vcbuild" in the directory
     containing the sources.
  3) Open an SDK command prommpt and type "msbuild" in the directory containing
     the sources.

  NB: You may need to copy the sources to a new directory as the default 
      install directory is write-protected.

How to Run
==========

AsyncCreateFile <targetFile> <timeout> <returnOnTimeout>

  targetfile   Path to open; this can be a local path or a network 
               path (eg. \\machine\share\file).

  timeout      Time to wait in millseconds before cancelling the open.

  returnOnTimeout   A non-zero value indicates that if the operation times out, 
                    the program should immediately return.  Otherwise it will
                    wait for the create request to complete (after attempting
                    cancellation) and output the final status of the operation.
