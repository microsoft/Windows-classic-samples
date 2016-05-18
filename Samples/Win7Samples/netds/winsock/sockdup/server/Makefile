#----- Include the PSDK's WIN32.MAK to pick up defines------------------------------------
!include <win32.mak>

#----- OUTDIR is defined in WIN32.MAK This is the name of the destination directory-------
all: $(OUTDIR)\sockdups.exe 

#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

LIBS = user32.lib ws2_32.lib

SOCKDUPS_OBJS= $(OUTDIR)\sockdups.obj 
	
#--------------------- EXE ---------------------------------------------------------------
#PLEASE REFER TO WIN32.MAK for the different Linker options WIN32.MAK provides

# Build rule for EXE
$(OUTDIR)\sockdups.exe:  $(OUTDIR) $(SOCKDUPS_OBJS) 
	$(link) $(ldebug) $(conflags) /PDB:$(OUTDIR)\program.pdb -out:$(OUTDIR)\sockdups.exe $(SOCKDUPS_OBJS) $(conlibs) $(LIBS)


.c{$(OUTDIR)}.obj:
  $(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

clean:
	$(CLEANUP)
