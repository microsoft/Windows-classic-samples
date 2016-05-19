TARGETOS=BOTH
!include <win32.mak>

objs_ws1=$(OUTDIR)\mcastws1.obj $(OUTDIR)\resolve.obj

all: $(OUTDIR) $(OUTDIR)\mcastws1.exe 

#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# Update the resource if necessary

# Update the object file if necessary

.c{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

# Update the executable file if necessary, and if so, add the resource back in.

$(OUTDIR)\mcastws1.exe: $(objs_ws1) $(common_objs) 
    $(link) $(linkdebug) $(conlflags) -out:$(OUTDIR)\mcastws1.exe $(objs_ws1) oldnames.lib kernel32.lib wsock32.lib



#--------------------- Clean Rule --------------------------------------------------------
# Rules for cleaning out those old files
clean:
        $(CLEANUP)
