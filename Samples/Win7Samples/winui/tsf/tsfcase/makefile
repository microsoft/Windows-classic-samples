#----- set the project name --------------------------------------------------------------
Proj = Case

#----- Include the PSDK's WIN32.MAK to pick up defines -----------------------------------
!include <win32.mak>

#----- OUTDIR is defined in WIN32.MAK This is the name of the destination directory ------
all: $(OUTDIR)\$(Proj).dll

LINK32_OBJS= \
	$(OUTDIR)\case.obj \
	$(OUTDIR)\dllmain.obj \
	$(OUTDIR)\editsink.obj \
	$(OUTDIR)\flipdoc.obj \
	$(OUTDIR)\flipsel.obj \
	$(OUTDIR)\globals.obj \
	$(OUTDIR)\hello.obj \
	$(OUTDIR)\keys.obj \
	$(OUTDIR)\langbar.obj \
	$(OUTDIR)\precomp.obj \
	$(OUTDIR)\register.obj \
	$(OUTDIR)\server.obj \
	$(OUTDIR)\snoop.obj \
	$(OUTDIR)\tmgrsink.obj
        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

#----- add the preprocessor definitions needed by this project ---------------------------

#----- set the libraries needed by this project ------------------------------------------
LINK_LIBS = $(dlllibs) $(guilibs) $(olelibs)

#--------------------- DLL ---------------------------------------------------------------
#PLEASE REFER TO WIN32.MAK for the different Linker options WIN32.MAK provides

# Build rule for DLL
$(OUTDIR)\$(Proj).dll: $(OUTDIR) $(LINK32_OBJS) $(OUTDIR)\Case.res
    $(link) $(ldebug) $(dllllflags) \
    $(LINK32_OBJS) $(OUTDIR)\Case.res $(LINK_LIBS) \
    -out:$(OUTDIR)\$(Proj).dll \
    -def:Case.def

#--------------------- Compiling C/CPP Files ---------------------------------------------
#PLEASE REFER TO WIN32.MAK for the different Compiler options WIN32.MAK provides

FILE=case

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

FILE=dllmain

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

FILE=editsink

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

FILE=flipdoc

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

FILE=flipsel

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

FILE=globals

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

FILE=hello

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

FILE=keys

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

FILE=langbar

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

FILE=precomp

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

FILE=register

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

FILE=server

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

FILE=snoop

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

FILE=tmgrsink

$(OUTDIR)\$(FILE).obj : .\$(FILE).cpp $(OUTDIR)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" .\$(FILE).cpp

#--------------------- Compiling Resource Files ------------------------------------------
#PLEASE REFER TO WIN32.MAK for the different Resource Compiler options WIN32.MAK provides

# Build rule for resource file
FILE=Case

$(OUTDIR)\$(FILE).res: .\$(FILE).rc $(OUTDIR)
    $(rc) $(rcflags) $(rcvars) /fo $(OUTDIR)\$(FILE).res .\$(FILE).rc

#--------------------- Clean Rule --------------------------------------------------------
# Rules for cleaning out those old files
clean:
        $(CLEANUP)
