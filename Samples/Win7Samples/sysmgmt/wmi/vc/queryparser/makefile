#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>

PROJ = QueryParser
all:    $(OUTDIR) $(OUTDIR)\$(PROJ).exe

$(OUTDIR):
     if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

OBJS	= $(OUTDIR)\QueryParser.obj

INCLUDES=$(INCLUDES); \

EXTRA_LIBS = $(EXTRA_LIBS) wbemuuid.lib

CFLAGS  = $(CFLAGS) -c -Od -Z7 -W3 -nologo \
 -DWIN32 -DWIN32ANSI -DSTRICT -DDEBUG /D "_WIN32_DCOM"


cc = cl $(CFLAGS)

.cpp{$(OUTDIR)}.obj:
    if not exist $(OUTDIR) md $(OUTDIR)
    $(cc) $< -Fo$(OUTDIR)\$(<B).obj

$(OUTDIR)\$(PROJ).exe : $(OBJS) $(DEFFILE)
     $(link) $(ldebug) /MACHINE:$(CPU) -out:$(OUTDIR)\$(PROJ).EXE  $(OBJS)  $(EXTRA_LIBS) $(olelibs)


##### Dependencies #####

methcli.obj   : methcli.cpp