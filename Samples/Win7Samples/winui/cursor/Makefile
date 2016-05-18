# Nmake macros for building Win32 & Win64 apps

!include <Win32.Mak>

all: $(OUTDIR) $(OUTDIR)\Cursor.Exe

$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# Update the resource if necessary

$(OUTDIR)\Cursor.Res: Cursor.Rc Cursor.h BullsEye.Cur
    $(rc) $(rcflags) $(rcvars) /fo $(OUTDIR)\cursor.res Cursor.Rc

# Update the object file if necessary

$(OUTDIR)\Cursor.Obj: Cursor.c Cursor.h
    $(cc) $(cflags) $(cvars) $(cdebug) /WX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" Cursor.c

# Update the executable file if necessary, and if so, add the resource back in.

$(OUTDIR)\Cursor.Exe: $(OUTDIR)\Cursor.Obj $(OUTDIR)\Cursor.Res Cursor.Def
    $(link) $(linkdebug) $(guiflags) -out:$(OUTDIR)\Cursor.Exe $(OUTDIR)\Cursor.Obj $(OUTDIR)\Cursor.Res $(guilibs)

clean:
        $(CLEANUP)
