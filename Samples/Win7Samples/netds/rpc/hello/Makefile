#*************************************************************#
#**                                                         **#
#**                 Microsoft RPC Examples                  **#
#**                   hello Application                     **#
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

all : helloc hellos

# Make the client side application helloc
helloc : helloc.exe
helloc.exe : helloc.obj hello_c.obj spn.obj
    $(link) $(linkdebug) $(conflags) -out:helloc.exe \
      helloc.obj hello_c.obj spn.obj \
      rpcrt4.lib ntdsapi.lib secur32.lib netapi32.lib $(conlibsdll)

# helloc main program
helloc.obj : helloc.c hello.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) $*.c

# helloc stub
hello_c.obj : hello_c.c hello.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) $*.c

# Make the server side application
hellos : hellos.exe
hellos.exe : hellos.obj hellop.obj hello_s.obj spn.obj
    $(link) $(linkdebug) $(conflags) -out:hellos.exe \
      hellos.obj hello_s.obj hellop.obj spn.obj \
      rpcrt4.lib ntdsapi.lib secur32.lib netapi32.lib $(conlibsdll)

# hello server main program
hellos.obj : hellos.c hello.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) $*.c

# remote procedures
hellop.obj : hellop.c hello.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) $*.c

# hellos stub file
hello_s.obj : hello_s.c hello.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) $*.c

# spn 
spn.obj : spn.c
   $(cc) $(cdebug) $(cflags) $(cvarsdll) $*.c

# Stubs and header file from the IDL file
hello.h hello_c.c hello_s.c : hello.idl hello.acf
    midl $(MIDL_OPTIMIZATION) -oldnames -cpp_cmd $(cc) -cpp_opt "-E"  hello.idl

# Clean up everything
cleanall : clean
    -del *.exe

# Clean up everything but the .EXEs
clean :
    -del *.obj
    -del *.map
    -del hello_c.c
    -del hello_s.c
    -del hello.h
