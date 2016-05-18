=======
SUMMARY
=======
  This sample demonstrates the use of the Traffic Control API.
  For as long as TCSample.exe is running, it allows the user to:
	- apply DSCP marking
        - apply 802.1p marking
        - throttle traffic

===================
SUPPORTED PLATFORMS
===================
  Windows XP or higher

=====
FILES
=====
  The directory contains the following files:

   File                  Description
   ----                  -----------
   ReadMe.txt            This file
   TCSample.c            The main implementation file
   TCSample.sln          The Visual Studio 2008 solution file
   TCSample.vcproj       The Visual Studio 2008 project file

===================
BUILDING THE SAMPLE
===================
  To build, type "msbuild TCSample.sln" from the Visual Studio 2008
  build command-line. Alternatively, the solution can also be built
  from within Visual Studio 2008 itself.

==================
RUNNING THE SAMPLE
==================

  To run the sample:
  ------------------
    1. Navigate to the directory that contains the new executable,
       using the command prompt. If using Windows Vista or later,
       please make sure the command prompt is elevated.
    2. Type TCSample.exe at the command prompt.

  Additional Run Steps:
  ---------------------
    1. TCSample.exe -help to list all supported command line
       parameters.

=============
SECURITY NOTE 
=============

  This sample is provided for educational purpose only to demonstrate
  how to use Traffic Control API. It is not intended to be used without
  modifications in a production environment and it has not been tested
  in a production environment. Microsoft assumes no liability for
  incidental or consequential damages should the sample code be used for 
  purposes other than as intended.
