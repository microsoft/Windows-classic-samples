!include <disable.mak>

#THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
#ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
#TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
#PARTICULAR PURPOSE.
#
#Copyright (C) 1996 - 2006.  Microsoft Corporation.  All rights reserved.
#

!include <win32.mak>

CPP          = $(cc)
CPPFLAGS     = $(cflags) $(cvars) $(cdebug)
LINKFLAGS    = $(ldebug) $(conlflags)

all: AuthzCli.Exe AuthzSvr.exe

# ----------------------------------------------------------------------------
#                      L I N K / R E S   C O M M A N D S
# ----------------------------------------------------------------------------

Authzcli.Exe: Authzcli.Obj common.obj
    @echo Linking ...
     $(link) $(LINKFLAGS) $** $(conlibs) advapi32.Lib user32.lib secur32.lib -out:$@

AuthzSvr.Exe: AuthzSvr.Obj common.obj
    @echo Linking ...
     $(link) $(LINKFLAGS) $** $(conlibs) advapi32.Lib user32.lib secur32.lib authz.lib -out:$@


# ----------------------------------------------------------------------------
#                           B U I L D   R U L E S
# ----------------------------------------------------------------------------

.c.Obj:
    @echo Compiling $<...
    $(CPP) $(CPPFLAGS) /c -I..\include $<



# ----------------------------------------------------------------------------
#                       D E P E N D E N C Y   R U L E S
# ----------------------------------------------------------------------------

AuthzCli.Obj : AuthzCli.c common.c common.h

AuthzSvr.Obj : AuthzSvr.c common.c common.h

common.Obj : common.c


