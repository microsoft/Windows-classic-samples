#*************************************************************#
#**                                                         **#
#**                 Microsoft RPC Examples                  **#
#**                   usrdef Application                    **#
#**       Copyright 1992 - 1999 Microsoft Corporation       **#
#**                                                         **#
#*************************************************************#

!include <Win32.Mak>

!if "$(CPU)" == "i386"
cflags = $(cflags) -D_CRTAPI1=_cdecl -D_CRTAPI2=_cdecl -DSECURITY_WIN32
!else
cflags = $(cflags) /W3 -D_CRTAPI1= -D_CRTAPI2= -DSECURITY_WIN32
!endif

cflags = $(cflags) -GS 

!if "$(CPU)" == "i386"
link = $(link) -SAFESEH
!endif

all : usrdefc usrdefs

# Make the usrdefc
usrdefc : usrdefc.exe
usrdefc.exe : usrdefc.obj usrdef_c.obj spn.obj
    $(link) $(linkdebug) $(conflags) -out:usrdefc.exe \
      usrdefc.obj usrdef_c.obj spn.obj\
      rpcrt4.lib ntdsapi.lib secur32.lib netapi32.lib $(conlibsdll)

# usrdefc main program
usrdefc.obj : usrdefc.c usrdef.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) $*.c

# usrdefc stub
usrdef_c.obj : usrdef_c.c usrdef.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) $*.c


# Make the usrdefs
usrdefs : usrdefs.exe
usrdefs.exe : usrdefs.obj usrdefp.obj usrdef_s.obj spn.obj
    $(link) $(linkdebug) $(conflags) -out:usrdefs.exe \
      usrdefs.obj usrdef_s.obj usrdefp.obj spn.obj\
      rpcrt4.lib ntdsapi.lib secur32.lib netapi32.lib $(conlibsdll)

# usrdefs main loop
usrdefs.obj : usrdefs.c usrdef.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) $*.c

# remote procedures
usrdefp.obj : usrdefp.c usrdef.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) $*.c

# usrdefs stub file
usrdef_s.obj : usrdef_s.c usrdef.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) $*.c

# spn 
spn.obj : spn.c
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# Stubs and header file from the IDL file
usrdef.h usrdef_c.c usrdef_s.c : usrdef.idl usrdef.acf
    midl $(MIDL_OPTIMIZATION) -oldnames -cpp_cmd $(cc) -cpp_opt "-E" usrdef.idl

# Clean up everything
cleanall : clean
    -del *.exe

# Clean up everything but the .EXEs
clean :
    -del *.obj
    -del *.map
    -del usrdef_c.c
    -del usrdef_s.c
    -del usrdef.h
