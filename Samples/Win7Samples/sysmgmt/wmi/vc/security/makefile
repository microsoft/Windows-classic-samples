#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>

PROJ = Security
all:    $(OUTDIR) $(OUTDIR)\$(PROJ).exe

$(OUTDIR):
     if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

OBJS	= 	$(OUTDIR)\Security.obj

EXTRA_LIBS = $(EXTRA_LIBS) wbemuuid.lib credui.lib


CFLAGS  = $(CFLAGS) -c -Ot -W3 -nologo  \
 -DWIN32 -DSTRICT /D "_WIN32_DCOM"  -D_UNICODE -Dunicode


cc = cl $(CFLAGS)

.cpp{$(OUTDIR)}.obj:
    if not exist $(OUTDIR) md $(OUTDIR)
    $(cc) $< -Fo$(OUTDIR)\$(<B).obj


$(OUTDIR)\$(PROJ).exe : $(OBJS) $(DEFFILE)
     $(link) $(ldebug) /MACHINE:$(CPU) -out:$(OUTDIR)\$(PROJ).EXE  $(OBJS)  $(EXTRA_LIBS) $(olelibs)

##### Dependencies #####

Security.obj   : Security.cpp
