# Nmake macros for building Windows 32-Bit apps

!include <win32.mak>

all: $(OUTDIR) $(OUTDIR)\ipxchat.exe

$(OUTDIR):
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# Update the resource if necessary

OBJS = $(OUTDIR)\ipxchat.obj $(OUTDIR)\about.obj $(OUTDIR)\dispatch.obj $(OUTDIR)\init.obj $(OUTDIR)\misc.obj $(OUTDIR)\winmain.obj $(OUTDIR)\listen.obj $(OUTDIR)\connect.obj

# Inference rules
.c{$(OUTDIR)}.obj:
   $(cc) $(cflags) $(cvars) $(cdebug) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**


# Dependencies

$(OUTDIR)\ipxchat.obj:  ipxchat.c
$(OUTDIR)\about.obj:    about.c
$(OUTDIR)\init.obj:     init.c 
$(OUTDIR)\dispatch.obj: dispatch.c 
$(OUTDIR)\misc.obj:     misc.c 
$(OUTDIR)\winmain.obj:  winmain.c 
$(OUTDIR)\listen.obj:   listen.c 
$(OUTDIR)\connect.obj:  connect.c 


$(OUTDIR)\ipxchat.res: ipxchat.rc globals.h
    rc -r $(rcvars) /fo $(OUTDIR)\ipxchat.res ipxchat.rc

$(OUTDIR)\ipxchat.exe: $(OBJS) $(OUTDIR)\ipxchat.res
   $(link) $(linkdebug) $(guiflags) -out:$(OUTDIR)\ipxchat.exe                           \
          $(OBJS) $(OUTDIR)\ipxchat.res $(guilibs) version.lib


clean:
       $(CLEANUP)

cleaner: clean
   del $(OUTDIR)\*.exe
