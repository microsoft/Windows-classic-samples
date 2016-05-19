# Nmake macros for building Win32 & Win64 apps

!IF "$(TARGETOS)" == "WINNT"

!include <Win32.Mak>

!IFNDEF NODEBUG
cflags = $(cflags) -DDBG_CERTSRV -DDBG
!ENDIF

PROJNAME = CertPSam
OBJFILES = 	$(OUTDIR)\CeLib.Obj \
		$(OUTDIR)\CeDebug.Obj \
		$(OUTDIR)\CeError.Obj \
		$(OUTDIR)\CeFormat.Obj \
		$(OUTDIR)\CertPSam.Obj \
		$(OUTDIR)\Module.Obj \
		$(OUTDIR)\Policy.obj \
		$(OUTDIR)\Request.obj \
		$(OUTDIR)\Pch.Obj \
		$(OUTDIR)\CertPSam_i.Obj \
		$(OUTDIR)\certPSam.Res \

EXTRA_LIBS = CertIdl.Lib Crypt32.Lib WinINet.Lib WinHttp.Lib

all: $(OUTDIR) clean CertPSam.Res $(OUTDIR)\$(PROJNAME).Dll

$(OUTDIR):
     if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CertPSam.Res: CertPSam.rc certPSam.tlb
    $(rc) $(rcvars) -I $(OUTDIR) -r -fo $(OUTDIR)\$*.Res $*.Rc

CertPSam.Tlb: CertPSam.Idl
   midl /h CertPSam.h /iid CertPSam_i.c /tlb $(OUTDIR)\CertPSam.Tlb CertPSam.Idl


.c{$(OUTDIR)}.Obj:
    $(cc) $(cdebug) $(cflags) $(cvarsdll) -I. -DUNICODE /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

.Cpp{$(OUTDIR)}.Obj:
    $(cc) $(cdebug) $(cflags) /EHsc $(cvarsdll) -I. -DUNICODE /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\$(PROJNAME).Dll: $(OBJFILES)
    $(link) $(linkdebug) $(dlllflags) /MACHINE:$(CPU) /PDB:$(OUTDIR)\$(PROJNAME).pdb  -out:$(OUTDIR)\$(PROJNAME).Dll /DEF:CertPSam.Def  $** $(EXTRA_LIBS) $(olelibsdll) 
    mt.exe -manifest $@.manifest -outputresource:$@;2

clean:
    del $(OUTDIR)\*.Obj $(OUTDIR)\*.Res $(OUTDIR)\*.pdb $(OUTDIR)\*.lib $(OUTDIR)\*.manifest $(OUTDIR)\*.dll  $(OUTDIR)\*.exp $(OUTDIR)\*.tlb CertPSam_i.c CertPSam.h
!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platform
!ENDIF
