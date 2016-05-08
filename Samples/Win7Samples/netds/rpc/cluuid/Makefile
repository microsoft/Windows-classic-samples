

#*************************************************************#
#**                                                         **#
#**                 Microsoft RPC Examples                  **#
#**                   cluuid Application                    **#
#**       Copyright 1992 - 1999 Microsoft Corporation       **#
#**                                                         **#
#*************************************************************#

!include <Win32.Mak>

!if "$(CPU)" == "i386"
cflags = $(cflags) -D_CRTAPI1=_cdecl -D_CRTAPI2=_cdecl -DSECURITY_WIN32
!else
cflags = $(cflags) -D_CRTAPI1= -D_CRTAPI2= -DSECURITY_WIN32
!endif

cflags = $(cflags) -GS

!if "$(CPU)" == "i386"
link = $(link) -SAFESEH
!endif

all : cluuidc cluuids

# Make the client side application cluuidc
cluuidc : cluuidc.exe
cluuidc.exe : cluuidc.obj cluuid_c.obj spn.obj
    $(link) $(linkdebug) $(conflags) -out:cluuidc.exe \
      cluuidc.obj cluuid_c.obj spn.obj\
      rpcrt4.lib ntdsapi.lib secur32.lib netapi32.lib $(conlibsdll)

# cluuidc main program
cluuidc.obj : cluuidc.c cluuid.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# cluuidc stub
cluuid_c.obj : cluuid_c.c cluuid.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# Make the server side application
cluuids : cluuids.exe
cluuids.exe : cluuids.obj cluuidp.obj cluuid_s.obj spn.obj
    $(link) $(linkdebug) $(conflags) -out:cluuids.exe \
      cluuids.obj cluuid_s.obj cluuidp.obj spn.obj\
      rpcrt4.lib ntdsapi.lib secur32.lib netapi32.lib $(conlibsdll)

# cluuid server main program
cluuids.obj : cluuids.c cluuid.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# remote procedures
cluuidp.obj : cluuidp.c cluuid.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# cluuids stub file
cluuid_s.obj : cluuid_s.c cluuid.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# spn 
spn.obj : spn.c
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# Stubs and header file from the IDL file
cluuid.h cluuid_c.c cluuid_s.c : cluuid.idl cluuid.acf
    midl $(MIDL_OPTIMIZATION) -oldnames -use_epv -no_cpp cluuid.idl

# Clean up everything
cleanall : clean
    -del *.exe

# Clean up everything but the .EXEs
clean :
    -del *.obj
    -del *.map
    -del cluuid_c.c
    -del cluuid_s.c
    -del cluuid.h
