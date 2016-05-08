# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
# TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (C) 1987-1999.  Microsoft Corporation.  All rights reserved.
#

!IF "$(TARGETOS)" == "WINNT"

!include <win32.mak>

incdirs = -I. 

all: subauth.dll

.c.obj:
  $(cc) $(incdirs) $(cflags) $(cvarsdll) $<

subauth.dll: subauth.obj
  $(link) $(dlllflags) $(guilibsdll) \
  $** -out:subauth.dll -def:subauth.def netapi32.lib
!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platform
!ENDIF
