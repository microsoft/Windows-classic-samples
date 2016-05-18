#----- set the project name --------------------------------------------------------------
Proj = Compart

#----- Include the PSDK's WIN32.MAK to pick up defines -----------------------------------
!include <win32.mak>

#----- OUTDIR is defined in WIN32.MAK This is the name of the destination directory ------
all: $(OUTDIR)\$(Proj).exe

LINK32_OBJS= \
	$(OUTDIR)\Compart.obj \
	$(OUTDIR)\Monitor.obj 
        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

#----- set the libraries needed by this project ------------------------------------------
LINK_LIBS = $(olelibs)

#--------------------- EXE ---------------------------------------------------------------
#PLEASE REFER TO WIN32.MAK for the different Linker options WIN32.MAK provides

# Build rule for EXE
$(OUTDIR)\$(Proj).exe:  $(OUTDIR) $(LINK32_OBJS) $(OUTDIR)\Compart.res
    $(link) $(ldebug) $(guiflags) /PDB:$(OUTDIR)\$(Proj).pdb -out:$(OUTDIR)\$(Proj).exe $(LINK32_OBJS) $(OUTDIR)\Compart.res $(LINK_LIBS) 

#--------------------- Compiling C/CPP Files ---------------------------------------------
#PLEASE REFER TO WIN32.MAK for the different Compiler options WIN32.MAK provides

SOURCE=.\Compart.cpp

$(OUTDIR)\Compart.obj : $(SOURCE) $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /WX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $(SOURCE)

SOURCE=.\Monitor.cpp

$(OUTDIR)\Monitor.obj : $(SOURCE) $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /WX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $(SOURCE)

#--------------------- Compiling Resource Files ------------------------------------------
#PLEASE REFER TO WIN32.MAK for the different Resource Compiler options WIN32.MAK provides

# Build rule for resource file
SOURCE=.\Compart.rc

$(OUTDIR)\Compart.res: $(SOURCE) $(OUTDIR)
    $(rc) $(rcflags) $(rcvars) /fo $(OUTDIR)\Compart.res $(SOURCE)

#--------------------- Clean Rule --------------------------------------------------------
# Rules for cleaning out those old files
clean:
        $(CLEANUP)
