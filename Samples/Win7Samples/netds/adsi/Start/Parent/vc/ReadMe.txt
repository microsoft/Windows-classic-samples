//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Parent VC Sample: Binding to an Object's Parent Using ADSI
//
//--------------------------------------------------------------------------

Description
===========
The Parent sample program binds to an object and then uses the IADs interface
to retrieve a binding string for the object's parent.

This sample uses the WinNT: provider and is suitable for Windows NT(R) 4.0
networks as well as Windows 2000 and later networks running Active Directory.

Sample Files
============
  *  makefile
  *  Parent.cpp
  *  Parent.sln
  *  Parent.vcproj
  *  StdAfx.cpp
  *  StdAfx.h

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the solution Parent.sln.
  2.  Open the source file Parent.Cpp.
  3.  Replace the domain name "INDEPENDENCE" with the appropriate domain
      name, such as FABRIKAM, and the user name "JJohnson" with an existing
      user in the domain in the following line.
        hr = ADsGetObject(L"WinNT://INDEPENDENCE/JJohnson",  \
                          IID_IADs, (void**) &pADs );
  4.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command "Parent.exe".

You can also run the sample by selecting Execute Parent.exe from
the Build menu.

Example Output
==============
If the sample executes successfully, there is no output.

How the Sample Works
====================
The sample uses the WinNT ADsPath and the IADs interface to perform
the bindings.

See Also
========
IADs interface
WinNT ADsPath
WinNT Binding String (ADsPath)

