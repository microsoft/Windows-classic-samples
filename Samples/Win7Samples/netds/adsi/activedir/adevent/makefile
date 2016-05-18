Proj = ADEvent

# pull in the master SDK makefile that defines all of the macros
#  and all of the build rules

!include <win32.mak>

all: $(OUTDIR) $(OUTDIR)\$(Proj).dll

$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# itemize all of the required Object files

Objs = $(OUTDIR)\$(Proj).obj

# build the Unicode version
cflags = $(cflags) -D UNICODE -D _UNICODE -D __UNICODE

.cpp{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**


$(OUTDIR)\$(Proj).dll: $(Objs) 
    $(link) $(ldebug) $(dllllflags) \
    $(Objs) $(dlllibs) $(olelibs) \
    -out:$(OUTDIR)\$(Proj).dll \
    -def:$(Proj).def


clean:
        $(CLEANUP)
