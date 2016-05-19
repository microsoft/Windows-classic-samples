#*************************************************************#
#**                                                         **#
#**                 Microsoft RPC Examples                  **#
#**                   umarsh Application                     **#
#**        Copyright 1992 - 1999 Microsoft Corporation      **#
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

all : umarshc umarshs

# Make the client
umarshc : umarshc.exe
umarshc.exe : umarshc.obj umarshcu.obj umarsh_c.obj spn.obj
    $(link) $(linkdebug) $(conflags) -out:umarshc.exe -map:umarshc.map \
      umarshc.obj umarshcu.obj umarsh_c.obj spn.obj \
      rpcrt4.lib ntdsapi.lib secur32.lib netapi32.lib $(conlibsdll)

# umarsh client main program
umarshc.obj : umarshc.c umarshc.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

umarshcu.obj : umarshcu.c umarshc.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# umarsh client stub
umarsh_c.obj : umarsh_c.c umarshc.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# Stubs, auxiliary and header file from the IDL file for the client side
umarshc.h umarsh_c.c : umarsh.idl umarshc.acf
    midl $(MIDL_OPTIMIZATION) -oldnames -ms_ext -no_cpp umarsh.idl -acf umarshc.acf -header umarshc.h -server none

# Make the server executable
umarshs : umarshs.exe
umarshs.exe : umarshs.obj umarshsu.obj umarshp.obj umarsh_s.obj spn.obj
    $(link) $(linkdebug) $(conflags) -out:umarshs.exe -map:umarshs.map \
      umarshs.obj umarshsu.obj umarsh_s.obj umarshp.obj spn.obj \
      rpcrt4.lib ntdsapi.lib secur32.lib netapi32.lib $(conlibsdll)

# umarsh server main
umarshs.obj : umarshs.c umarshs.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

umarshsu.obj : umarshsu.c umarshs.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# remote procedures
umarshp.obj : umarshp.c umarshs.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# umarsh server stub file
umarsh_s.obj : umarsh_s.c umarshs.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) $*.c

# spn 
spn.obj : spn.c
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# Stubs, auxiliary and header file from the IDL file
umarshs.h umarsh_s.c : umarsh.idl umarshs.acf
    midl $(MIDL_OPTIMIZATION) -oldnames -ms_ext -no_cpp umarsh.idl -acf umarshs.acf -header umarshs.h -client none

# Clean up everything
cleanall : clean
    -del *.exe

# Clean up everything but the .EXEs
clean :
    -del *.obj
    -del *.map
    -del umarsh_c.c
    -del umarsh_s.c
    -del umarshc.h
    -del umarshs.h
