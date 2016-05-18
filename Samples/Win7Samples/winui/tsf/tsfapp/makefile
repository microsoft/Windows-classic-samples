#----- set the project name --------------------------------------------------------------
Proj = TSFApp

#----- Include the PSDK's WIN32.MAK to pick up defines -----------------------------------
!include <win32.mak>

#----- OUTDIR is defined in WIN32.MAK This is the name of the destination directory ------
all: $(OUTDIR)\$(Proj).exe

LINK32_OBJS= \
	$(OUTDIR)\Context.obj \
	$(OUTDIR)\DataObj.obj \
	$(OUTDIR)\FuncProv.obj \
	$(OUTDIR)\TextStor.obj \
	$(OUTDIR)\TSFApp.obj \
	$(OUTDIR)\TSFEdit.obj \
	$(OUTDIR)\TSFWnd.obj \
	$(OUTDIR)\Persist.obj \
	$(OUTDIR)\PropLdr.obj \
	$(OUTDIR)\Test.obj
        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

#----- add the preprocessor definitions needed by this project ---------------------------
cflags = $(cflags) -DUNICODE -D_UNICODE

#----- set the libraries needed by this project ------------------------------------------
LINK_LIBS = $(olelibs) comctl32.lib

#--------------------- EXE ---------------------------------------------------------------
#PLEASE REFER TO WIN32.MAK for the different Linker options WIN32.MAK provides

# Build rule for EXE
$(OUTDIR)\$(Proj).exe:  $(OUTDIR) $(LINK32_OBJS) $(OUTDIR)\TSFApp.res
    $(link) $(ldebug) $(guiflags) /PDB:$(OUTDIR)\$(Proj).pdb -out:$(OUTDIR)\$(Proj).exe $(LINK32_OBJS) $(OUTDIR)\TSFApp.res $(LINK_LIBS) 

#--------------------- Compiling C/CPP Files ---------------------------------------------
#PLEASE REFER TO WIN32.MAK for the different Compiler options WIN32.MAK provides

FILE=Context

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /WX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

FILE=DataObj

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /WX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

FILE=FuncProv

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /WX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

FILE=TextStor

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /WX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

FILE=TSFApp

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /WX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

FILE=TSFEdit

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /WX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

FILE=TSFWnd

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /WX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

FILE=Persist

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /WX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

FILE=PropLdr

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /WX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

FILE=Test

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /WX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

#--------------------- Compiling Resource Files ------------------------------------------
#PLEASE REFER TO WIN32.MAK for the different Resource Compiler options WIN32.MAK provides

# Build rule for resource file
FILE=TSFApp

$(OUTDIR)\$(FILE).res: .\$(FILE).rc $(OUTDIR)
    $(rc) $(rcflags) $(rcvars) /fo $(OUTDIR)\$(FILE).res .\$(FILE).rc

#--------------------- Clean Rule --------------------------------------------------------
# Rules for cleaning out those old files
clean:
        $(CLEANUP)
