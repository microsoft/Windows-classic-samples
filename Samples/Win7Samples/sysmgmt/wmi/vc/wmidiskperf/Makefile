#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>

#TARGET  = WMIDiskPerf
#OUTFILE = WMIDiskPerf.exe
PROJ = WMIDiskPerf
all:    $(OUTDIR) $(OUTDIR)\$(PROJ).exe

$(OUTDIR):
     if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"


CFLAGS  = $(CFLAGS) /nologo -I..\..\..\include /MDd /W3 /Gm /EHsc /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_WIN32_DCOM" /c


LINK_OBJS = $(OUTDIR)\WMIDiskPerf.obj \
	$(OUTDIR)\dpdetail.obj 


EXTRA_LIBS = $(EXTRA_LIBS) wbemuuid.lib


cc = cl $(CFLAGS)

.cpp{$(OUTDIR)}.obj:
    if not exist $(OUTDIR) md $(OUTDIR)
    $(cc) $< -Fo$(OUTDIR)\$(<B).obj

$(OUTDIR)\$(PROJ).exe : $(LINK_OBJS) $(DEFFILE)
	$(link) $(ldebug) /MACHINE:$(CPU) -out:$(OUTDIR)\$(PROJ).EXE  $(LINK_OBJS)  $(EXTRA_LIBS) $(olelibs)

##### Dependencies #####

WMIDiskPerf.obj : WMIDiskPerf.cpp
dpdetail.obj : dpdetail.cpp

