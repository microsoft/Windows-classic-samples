# Include the common makefile
!include <win32.mak>

# OUTDIR is the output directory, and is defined in win32.mak.
all: $(OUTDIR)\wiassamp.exe
 
# If OUTDIR does not exist, create it
"$(OUTDIR)" :
	if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)
 
# Build rule for the EXE
$(OUTDIR)\wiassamp.exe:  $(OUTDIR) $(OUTDIR)\wiassamp.obj
	$(link) $(ldebug) $(conflags) /PDB:$(OUTDIR)\wiassamp.pdb -out:$(OUTDIR)\wiassamp.exe $(OUTDIR)\wiassamp.obj $(conlibs) wiaguid.lib ole32.lib

# embedd the manifest
    if exist $@.manifest mt.exe -manifest $@.manifest -outputresource:$@;1

# Inference rule for building object files from cpp files
.cpp{$(OUTDIR)}.obj:
	$(cc) $(cdebug) $(cflags) $(cvarsdll) /WX /D_UNICODE /DUNICODE /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

# Remove created files
clean:
	$(CLEANUP)

