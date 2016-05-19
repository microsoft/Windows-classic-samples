# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
# TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (C) 1993-1999.  Microsoft Corporation.  All rights reserved.
#

!IF "$(TARGETOS)" == "WINNT"
!include <win32.mak>

all: sampssp.dll

# Define the compiler flags conditional on NODEBUG
!IFDEF NODEBUG
sampsspflags= $(cflags) -DUNICODE -D_UNICODE
!ELSE
sampsspflags= $(cflags) -DUNICODE -D_UNICODE -DDBG
!ENDIF


# Update the resource if necessary
sampssp.res: res.rc
  rc -r -fo sampssp.res res.rc

# Inference rule for updating the object files
.c.obj:
  $(cc) $(cdebug) $(cvarsdll) $(sampsspflags) $*.c


# Update the DLL
sampssp.DLL:             \
          stubs.obj    \
          init.obj     \
          sampssp.res
  $(link) $(ldebug)                  \
          $(dlllflags)               \
          -ignore:4078               \
          -subsystem:console         \
          -def:sampssp.def           \
          -out:$*.dll                \
          $**                        \
          $(guilibsdll)

!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platform
!ENDIF
