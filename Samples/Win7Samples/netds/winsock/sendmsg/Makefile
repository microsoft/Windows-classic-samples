#----- Include the PSDK's WIN32.MAK to pick up defines ------------------------------------
!include <win32.mak>

!IF "$(TARGETOS)" == "WINNT"

#----- OUTDIR is defined in WIN32.MAK This is the name of the destination directory-------
all: $(OUTDIR)\SendMsg.exe

#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

#--------------------- EXE ---------------------------------------------------------------
#PLEASE REFER TO WIN32.MAK for the different Linker options WIN32.MAK provides

# Build rule for DLL
$(OUTDIR)\sendmsg.exe:  $(OUTDIR) $(OUTDIR)\sendmsg.obj
	$(link) $(ldebug) $(conflags) /PDB:$(OUTDIR)\sendmsg.pdb -out:$(OUTDIR)\sendmsg.exe $(OUTDIR)\sendmsg.obj $(conlibs)

SOURCE=.\sendmsg.cpp

$(OUTDIR)\sendmsg.obj : $(SOURCE) $(OUTDIR)
    $(cc) $(cdebug) $(cflags) /WX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $(SOURCE)

clean:
	$(CLEANUP)

!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platforms
!ENDIF

