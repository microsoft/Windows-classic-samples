TARGETOS=BOTH

#----- Include the PSDK's WIN32.MAK to pick up defines------------------------------------
!include <win32.mak>

#----- OUTDIR is defined in WIN32.MAK This is the name of the destination directory-------
all: $(OUTDIR)\iphdrinc.exe


#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

#--------------------- Compiling C/CPP Files ---------------------------------------------
#PLEASE REFER TO WIN32.MAK for the different Compiler options WIN32.MAK provides

SOURCE=.\rawudp.c .\resolve.c

$(OUTDIR)\rawudp.obj : $(SOURCE) $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /WX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $(SOURCE)

$(OUTDIR)\resolve.obj : $(SOURCE) $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /WX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $(SOURCE)

#--------------------- EXE ---------------------------------------------------------------
#PLEASE REFER TO WIN32.MAK for the different Linker options WIN32.MAK provides

# Build rule for EXE
$(OUTDIR)\iphdrinc.exe:  $(OUTDIR) $(OUTDIR)\rawudp.obj
    $(link) $(ldebug) $(conflags) /PDB:$(OUTDIR)\iphdrinc.pdb -out:$(OUTDIR)\iphdrinc.exe $(OUTDIR)\rawudp.obj $(OUTDIR)\resolve.obj $(conlibs)


#--------------------- Clean Rule --------------------------------------------------------
# Rules for cleaning out those old files
clean:
        $(CLEANUP)

