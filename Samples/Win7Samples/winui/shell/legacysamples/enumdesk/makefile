# These are "extra" libs

libs = shell32.lib comctl32.lib

# pull in the master SDK makefile that defines all of the macros
#  and all of the build rules

_WIN32_IE = 0x0500

!include <win32.mak>

#----- OUTDIR is defined in WIN32.MAK This is the name of the destination directory-------
all: $(OUTDIR)\enumdesk.exe

#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

LINK32_OBJS= \
	$(OUTDIR)\EnumDesk.obj \
	$(OUTDIR)\List.obj \
	$(OUTDIR)\Tree.obj \
	$(OUTDIR)\EnumDesk.res
	

# itemize all of the required include files

Includes =  EnumDesk.h  \
            List.h  \
            Tree.h  \
            resource.h

SOURCE=.\EnumDesk.cpp

$(OUTDIR)\EnumDesk.obj : $(SOURCE) $(OUTDIR) $(Includes)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /WX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $(SOURCE)

SOURCE=.\list.cpp

$(OUTDIR)\list.obj : $(SOURCE) $(OUTDIR) $(Includes)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /WX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $(SOURCE)

SOURCE=.\tree.cpp

$(OUTDIR)\tree.obj : $(SOURCE) $(OUTDIR) $(Includes)
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /WX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $(SOURCE)

# Build rule for resource file
SOURCE=.\enumdesk.rc

$(OUTDIR)\enumdesk.res: $(SOURCE) $(OUTDIR) $(Includes) split.cur
    $(rc) $(rcflags) $(rcvars) /fo $(OUTDIR)\enumdesk.res $(SOURCE)


# Build rule for EXE
$(OUTDIR)\enumdesk.exe:  $(OUTDIR) $(LINK32_OBJS)
    $(link) $(ldebug) $(guiflags) /PDB:$(OUTDIR)\enumdesk.pdb -out:$(OUTDIR)\enumdesk.exe $(LINK32_OBJS) $(olelibs) shell32.lib comctl32.lib

#--------------------- Clean Rule --------------------------------------------------------
# Rules for cleaning out those old files
clean:
        $(CLEANUP)
