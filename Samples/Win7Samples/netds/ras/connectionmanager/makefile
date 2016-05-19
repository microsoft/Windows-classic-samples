#----- Include the PSDK's WIN32.MAK to pick up defines------------------------------------
!include <win32.mak>

#----- OUTDIR is defined in WIN32.MAK This is the name of the destination directory-------
all: $(OUTDIR)\cmsample.dll

LINK32_OBJS= \
	$(OUTDIR)\cmsample.obj
        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# Generic rule for building ALL CPP files and placing their OBJ's in the OUTDIR
.cpp{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**
    
#--------------------- DLL ---------------------------------------------------------------
#PLEASE REFER TO WIN32.MAK for the different Linker options WIN32.MAK provides

# Build rule for DLL
$(OUTDIR)\cmsample.dll: $(OUTDIR) $(LINK32_OBJS)
    $(link) $(ldebug) $(dlllflags) \
    $(LINK32_OBJS) $(dlllibs) $(guilibs) \
    -out:$(OUTDIR)\cmsample.dll \
    -def:cmsample.def

#--------------------- Clean Rule --------------------------------------------------------
# Rules for cleaning out those old files
clean:
        $(CLEANUP)

cleanall:
	$(CLEANUP)
