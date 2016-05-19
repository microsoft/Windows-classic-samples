# Define NODEBUG to build DIALER without debugging information.

!IF "$(TARGETOS)" == "WINNT"

#----- Include the PSDK's WIN32.MAK to pick up defines ------------------------------------
!include <win32.mak>

cflags=$(cflags) -DWIN32_LEAN_AND_MEAN

all: $(OUTDIR)\dialer.exe

#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

#--------------------- EXE ---------------------------------------------------------------
#PLEASE REFER TO WIN32.MAK for the different Linker options WIN32.MAK provides

LIBS = tapi32.lib version.lib

# Build rule for EXE
$(OUTDIR)\dialer.exe:  $(OUTDIR) $(OUTDIR)\dialer.obj $(OUTDIR)\dialer.res
    $(link) $(ldebug) $(guiflags) /PDB:$(OUTDIR)\dialer.pdb -out:$(OUTDIR)\dialer.exe $(OUTDIR)\dialer.obj $(OUTDIR)\dialer.res $(LIBS) $(guilibsmt)
    if exist $(OUTDIR)\dialer.exe.manifest mt.exe -manifest $(OUTDIR)\dialer.exe.manifest -outputresource:$(OUTDIR)\dialer.exe;1

SOURCE=.\dialer.c

$(OUTDIR)\dialer.obj : $(SOURCE) $(OUTDIR) resource.h dialer.h
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $(SOURCE)

#--------------------- Compiling Resource Files ------------------------------------------
#PLEASE REFER TO WIN32.MAK for the different Resource Compiler options WIN32.MAK provides

# Build rule for resource file
SOURCE=.\dialer.rc

$(OUTDIR)\dialer.res: $(SOURCE) $(OUTDIR)
    $(rc) $(rcflags) $(rcvars) /fo $(OUTDIR)\dialer.res $(SOURCE)


    
clean:
	$(CLEANUP)
	
!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platform
!ENDIF
