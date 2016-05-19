#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>


PROJ = consumer
all:    $(OUTDIR) $(OUTDIR)\$(PROJ).exe

$(OUTDIR):
     if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"



OBJS = $(OUTDIR)\consumer.obj \



#---------------------------------------------------------------------


!if "$(CPU)"=="i386"
cc =  \
    cl -c -Od -D_MT -D_CONSOLE -DWIN32 -Z7 \
   -W3 -Di386=1 -D_X86_=1 -I.
!else
cc =  \
    cl -c -Od -D_MT -D_CONSOLE -DWIN32 -Z7 \
   -W3 -I.
!endif


EXTRA_LIBS = $(EXTRA_LIBS) wbemuuid.lib

.cpp{$(OUTDIR)}.obj:
    if not exist $(OUTDIR) md $(OUTDIR)
    $(cc) $< -Fo$(OUTDIR)\$(<B).obj

$(OUTDIR)\$(PROJ).exe : $(OBJS) $(DEFFILE)
     $(link) $(ldebug) /MACHINE:$(CPU) -out:$(OUTDIR)\$(PROJ).EXE  $(OBJS)  $(EXTRA_LIBS) $(olelibs)