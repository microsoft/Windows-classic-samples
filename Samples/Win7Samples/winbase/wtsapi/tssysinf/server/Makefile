# Nmake macros for building Win32 & Win64 apps
# Copyright 1999 Microsoft Corporation

!Include <Win32.Mak>

all: $(OUTDIR) $(OUTDIR)\SysInf_S.Exe

$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# Update the object file if necessary

$(OUTDIR)\SysInf_S.Obj: SysInf_S.c
    $(cc) $(cflags) $(cvarsdll) $(cdebug) -I..\Inc /WX /Fo"$(OUTDIR)\\" SysInf_S.c

# Update the resources if necessary

$(OUTDIR)\SysInf_S.Res: SysInf_S.Rc
    $(rc) $(rcvars) -r /fo $(OUTDIR)\sysinf_s.res SysInf_S.Rc

# Update the dynamic link library

$(OUTDIR)\SysInf_S.Exe: $(OUTDIR)\SysInf_S.Obj $(OUTDIR)\sysinf_s.res
    $(link) $(ldebug) $(conlflags) $(conlibsdll) \
    -out:$(OUTDIR)\SysInf_S.Exe \
    $(OUTDIR)\SysInf_S.Obj $(OUTDIR)\sysinf_s.res WTSAPI32.Lib User32.Lib

clean:
        $(CLEANUP)
