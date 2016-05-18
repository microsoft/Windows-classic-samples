# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (C) 1998-1999 Microsoft Corporation.  All Rights Reserved.

!IF "$(TARGETOS)" == "WINNT"

!include <win32.mak>


PROJ = klist

all: $(PROJ).exe

PROJ_OBJS = klist.obj

#Dependencies
klist.obj: klist.c


.c.obj:
   $(cc) $(cflags) $(cdebug) $(cvars) $<

$(PROJ).exe: $(PROJ_OBJS)
   $(link) $(conlflags) $(ldebug) $(PROJ_OBJS)\
   -out:$(PROJ).exe                           \
   $(conlibs) user32.lib secur32.lib


clean:
   del *.bak
   del *.obj

cleaner: clean
   del *.exe

!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platform
!ENDIF
