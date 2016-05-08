# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
# TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (C) 1993-1999.  Microsoft Corporation.  All rights reserved.
#
# Nmake macros for building Windows 32-Bit apps
#

!include <Win32.Mak>

all: RegExplorer.exe

# Update the resource if necessary

RegExplorer.res: RegExplorer.rc RegExplorer.h
    rc $(rcflags) $(rcvars) -r -fo RegExplorer.res RegExplorer.rc

# Update the object file if necessary

RegExplorer.obj: RegExplorer.c RegExplorer.h
    $(cc) $(cdebug) $(cflags) $(cvars) RegExplorer.c

# Update the executable file if necessary, and if so, add the resource back in.

RegExplorer.exe: RegExplorer.obj RegExplorer.res
    $(link) $(ldebug) $(guilflags) -out:RegExplorer.exe RegExplorer.obj RegExplorer.res $(guilibs) advapi32.lib
