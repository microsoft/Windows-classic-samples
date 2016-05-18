#*************************************************************#
#**                                                         **#
#**                 Microsoft RPC Examples                  **#
#**                   dunion Application                    **#
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
all : dunionc dunions

# Make the dunionc
dunionc : dunionc.exe
dunionc.exe : dunionc.obj dunion_c.obj spn.obj
    $(link) $(linkdebug) $(conflags) -out:dunionc.exe \
      dunionc.obj dunion_c.obj spn.obj\
      rpcrt4.lib ntdsapi.lib secur32.lib netapi32.lib $(conlibsdll)

# dunionc main program
dunionc.obj : dunionc.c dunion.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# dunionc stub
dunion_c.obj : dunion_c.c dunion.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# Make the dunions
dunions : dunions.exe
dunions.exe : dunions.obj dunionp.obj dunion_s.obj spn.obj
    $(link) $(linkdebug) $(conflags) -out:dunions.exe \
      dunions.obj dunion_s.obj dunionp.obj spn.obj\
      rpcrt4.lib ntdsapi.lib secur32.lib netapi32.lib $(conlibsdll)

# dunions main loop
dunions.obj : dunions.c dunion.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# remote procedures
dunionp.obj  : dunionp.c dunion.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# dunions stub file
dunion_s.obj : dunion_s.c dunion.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# spn 
spn.obj : spn.c
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# Stubs and header file from the IDL file
dunion.h dunion_c.c dunion_s.c : dunion.idl dunion.acf
    midl $(MIDL_OPTIMIZATION) -oldnames -no_cpp dunion.idl

# Clean up everything
cleanall : clean
    -del *.exe

# Clean up everything but the .EXEs
clean :
    -del *.obj
    -del *.map
    -del dunion_c.c
    -del dunion_s.c
    -del dunion.h
