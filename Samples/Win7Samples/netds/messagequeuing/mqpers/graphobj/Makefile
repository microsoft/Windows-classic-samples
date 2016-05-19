#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>

PROJ = GraphObj
all:    $(OUTDIR) $(OUTDIR)\$(PROJ).dll

$(OUTDIR):
     if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

EXTRA_LIBS = odbccp32.lib odbc32.lib shell32.lib advapi32.lib

LINK32_OBJS = $(OUTDIR)\stdafx.obj \
              $(OUTDIR)\point.obj \
              $(OUTDIR)\line.obj \
              $(OUTDIR)\graphobj.obj \
              $(OUTDIR)\dlldatax.obj \
              $(OUTDIR)\graphobj.res

$(OUTDIR)\graphobj.h : graphobj.idl
     $(midl) $(MIDL_OPTIMIZATION) /h $(OUTDIR)\graphobj.h /tlb $(OUTDIR)\graphobj.tlb /iid $(OUTDIR)\graphobj_i.c graphobj.idl

$(OUTDIR)\stdafx.obj:     stdafx.cpp
     $(cc) $(cflags) $(cdebug) $(cvarsdll) /DUNICODE /D_UNICODE /D_USRDLL /Yc"stdafx.h" /Fp"$(OUTDIR)\\" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /I$(OUTDIR) stdafx.cpp

.cpp{$(OUTDIR)}.obj:
     $(cc) $(cflags) $(cdebug) $(cvarsdll) /DUNICODE /D_UNICODE /D_USRDLL /Yu"stdafx.h" /Fp"$(OUTDIR)\\" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /I$(OUTDIR) $**

.c{$(OUTDIR)}.obj:
     $(cc) $(cflags) $(cdebug) $(cvarsdll) /DUNICODE /D_UNICODE /D_USRDLL /Fp"$(OUTDIR)\\" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /I$(OUTDIR) $**

$(OUTDIR)\graphobj.res:
     $(rc) $(rcflags) $(rcvars) /Fo$(OUTDIR)\graphobj.res /I$(OUTDIR) graphobj.rc

$(OUTDIR)\$(PROJ).dll:   $(OUTDIR)\graphobj.h $(LINK32_OBJS)
     $(link) $(ldebug) $(guilflags) $(dlllflags) /MACHINE:$(CPU) /PDB:$(OUTDIR)\$(PROJ).pdb -out:$(OUTDIR)\$(PROJ).dll /DEF:graphobj.def  $(LINK32_OBJS) $(EXTRA_LIBS) $(baselibs) $(olelibs) 

clean:
     $(CLEANUP)
