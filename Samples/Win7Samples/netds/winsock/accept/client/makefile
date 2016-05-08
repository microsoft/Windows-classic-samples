#----- Include the PSDK's WIN32.MAK to pick up defines------------------------------------
!include <win32.mak>

#----- OUTDIR is defined in WIN32.MAK This is the name of the destination directory-------
all: $(OUTDIR)\testacc.exe

#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

LIBS = user32.lib ws2_32.lib

testacc_OBJS= $(OUTDIR)\testaccept.obj 

.cpp{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

#--------------------- EXE ---------------------------------------------------------------
#PLEASE REFER TO WIN32.MAK for the different Linker options WIN32.MAK provides

# Build rule for EXE

$(OUTDIR)\testacc.exe:  $(OUTDIR) $(testacc_OBJS) 
	$(link) $(ldebug) $(conflags) /PDB:$(OUTDIR)\testacc.pdb -out:$(OUTDIR)\testacc.exe $(testacc_OBJS) $(conlibs) $(LIBS)

clean:
	$(CLEANUP)
