#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>

#
# MAKEFILE
# Sample WINMGMT SamplCli High Performance Client
#
# Copyright (c)Microsoft Corporation, All Rights Reserved
#



PROJ = samplcli
all:    $(OUTDIR) $(OUTDIR)\$(PROJ).exe

$(OUTDIR):
     if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

OBJS	= 	$(OUTDIR)\SamplCli.obj\
		$(OUTDIR)\RefClint.obj

INCLUDES=$(INCLUDES); \

EXTRA_LIBS = $(EXTRA_LIBS) wbemuuid.lib

CFLAGS  = $(CFLAGS) -nologo -c -Od -Z7 -W3 -DWIN32 -D_DEBUG -D_CONSOLE -D_MBCS


cc = cl $(CFLAGS)

.cpp{$(OUTDIR)}.obj:
    if not exist $(OUTDIR) md $(OUTDIR)
    $(cc) $< -Fo$(OUTDIR)\$(<B).obj

$(OUTDIR)\$(PROJ).exe : $(OBJS) $(DEFFILE)
     $(link) $(ldebug) /MACHINE:$(CPU) -out:$(OUTDIR)\$(PROJ).EXE  $(OBJS)  $(EXTRA_LIBS) $(olelibs)


##### Dependencies #####

SamplCli.obj	: SamplCli.cpp
RefClint.obj	: RefClint.cpp RefClint.h