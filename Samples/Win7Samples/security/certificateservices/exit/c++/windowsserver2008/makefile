# Nmake macros for building Windows 32-Bit apps

!IF "$(TARGETOS)" == "WINNT"

!include <win32.mak>

!IFNDEF NODEBUG
cflags = $(cflags) -DDBG_CERTSRV -DDBG
!ENDIF

PROJNAME = CertXSam
OBJFILES = 	$(OUTDIR)\ceDebug.Obj \
		$(OUTDIR)\ceError.Obj \
		$(OUTDIR)\ceLib.Obj \
		$(OUTDIR)\CertXSam.Obj \
		$(OUTDIR)\Exit.Obj \
		$(OUTDIR)\Module.Obj \
		$(OUTDIR)\pch.Obj \
		$(OUTDIR)\CertXSam_i.Obj \
		$(OUTDIR)\CertXSam.Res \

EXTRA_LIBS = Certidl.Lib Crypt32.Lib


all: $(OUTDIR) clean CertXSam.Res $(OUTDIR)\$(PROJNAME).Dll  

$(OUTDIR):
     if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CertXSam.Res: CertXSam.Rc CertXSam.tlb
    $(rc) $(rcvars) -I $(OUTDIR) -r -fo $(OUTDIR)\$*.Res $*.Rc

CertXSam.tlb: CertXSam.Idl
    midl /h CertXSam.h /iid CertXSam_i.c /tlb $(OUTDIR)\CertXSam.tlb CertXSam.Idl 

.c{$(OUTDIR)}.Obj:
    $(cc) $(cdebug) $(cflags) $(cvarsdll) -I. -DUNICODE /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

.Cpp{$(OUTDIR)}.Obj:
    $(cc) $(cdebug) $(cflags) /EHsc $(cvarsdll) -I. -DUNICODE /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**


$(OUTDIR)\$(PROJNAME).Dll: $(OBJFILES)
    $(link) $(linkdebug) $(dlllflags) /MACHINE:$(CPU) /PDB:$(OUTDIR)\$(PROJNAME).pdb  -out:$(OUTDIR)\$(PROJNAME).Dll /DEF:CertXSam.Def $** $(olelibsdll) $(EXTRA_LIBS)
    mt.exe -manifest $@.manifest -outputresource:$@;2

clean:
     del $(OUTDIR)\*.Obj $(OUTDIR)\*.Res $(OUTDIR)\*.pdb $(OUTDIR)\*.lib $(OUTDIR)\*.manifest $(OUTDIR)\*.dll  $(OUTDIR)\*.exp $(OUTDIR)\*.tlb CertXSam_i.c CertXSam.h
!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platform
!ENDIF
