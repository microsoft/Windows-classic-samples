!IF ("$(TARGETOS)" == "WINNT") && ("$(APPVER)" >= "6.0")

!IF ("$(CPU)" == "IA64") 
!MESSAGE Sorry, this sample is not supported on the IA64 platform!

!ELSE
# Include the common makefile
!include <win32.mak>
PRECOMP =  scanprofiles.h ScanProfile.h utils.h

PROJECT = ScanProfile

!ifndef MIDL
MIDL = midl
!endif

MIDLFLAGS = /Oicf -char unsigned -ms_ext -c_ext -error all   \
            -robust -cpp_cmd $(CC) -DMSPLOG -DUNICODE        \
            -D_WCHAR_T_DEFINED

all: $(OUTDIR) scanprofiles.h $(OUTDIR)\ScanProfile.exe
 

#Run MIDL compiler to generate the header
SCANPROFILESIDL = "$(MSSDK)\Include\scanprofiles.idl"

scanprofiles.h : $(SCANPROFILESIDL)  
    $(MIDL) $(MIDLFLAGS) $(SCANPROFILESIDL) /h scanprofiles.h


# OUTDIR is the output directory, and is defined in win32.mak.


# If OUTDIR does not exist, create it
$(OUTDIR) :
	if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)
 
# Build rule for the EXE
$(OUTDIR)\ScanProfile.exe: scanprofiles.h  $(OUTDIR) $(OUTDIR)\ScanProfile.obj $(OUTDIR)\utils.obj 
	$(link) $(ldebug) $(conflags) /PDB:$(OUTDIR)\ScanProfile.pdb -out:$(OUTDIR)\ScanProfile.exe $(OUTDIR)\ScanProfile.obj $(OUTDIR)\utils.obj $(conlibs) wiaguid.lib ole32.lib oleaut32.lib

# embedd the manifest
    if exist $@.manifest mt.exe -manifest $@.manifest -outputresource:$@;1

# Inference rule for building object files from cpp files
.cpp{$(OUTDIR)}.obj:
	$(cc) $(cdebug) $(cflags) $(cvarsdll) /WX /D_UNICODE /DUNICODE /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

    

"$(OUTDIR)\$(PRECOMP:.h=.obj)":
	@$(cc) $(cdebug) $(cflags) $(cvarsdll) /WX /D_UNICODE /DUNICODE /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /Fp"$(OUTDIR)\$(PROJECT).pch" /Yc"$(PRECOMP)" $?

# Remove created files
clean:
	$(CLEANUP)
    del scanprofiles.tlb scanprofiles.h scanprofiles_i.c scanprofiles_p.c dlldata.c



!ENDIF

!ELSE
!MESSAGE Sorry this sample only builds for the Windows Vista Platform or higher!
!ENDIF

