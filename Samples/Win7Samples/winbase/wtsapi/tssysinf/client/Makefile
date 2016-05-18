# Nmake macros for building Win32 & Win64 apps
# Copyright Microsoft Corporation

!Include <Win32.Mak>

all: $(OUTDIR) $(OUTDIR)\SysInf_C.Dll

$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# Update the object file if necessary

$(OUTDIR)\SysInf_C.Obj: SysInf_C.c
    $(cc) $(cflags) $(cvarsdll) $(cdebug) -I..\Inc /WX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" SysInf_C.c

# Update the resources if necessary

$(OUTDIR)\SysInf_C.Res: SysInf_C.Rc
    $(rc) $(rcvars) -r /fo $(OUTDIR)\sysinf_c.res SysInf_C.Rc

# Update the dynamic link library

$(OUTDIR)\SysInf_C.Dll: $(OUTDIR)\SysInf_C.Obj SysInf_C.Def
    $(link) $(linkdebug) $(dlllflags)       \
    -base:0x1C000000 \
    -out:$(OUTDIR)\SysInf_C.Dll \
    -def:SysInf_C.def $(OUTDIR)\SysInf_C.Obj $(guilibsdll)

MSC_WARNING_LEVEL=/WX /W3

clean:
        $(CLEANUP)
