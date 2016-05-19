#*************************************************************#
#**                                                         **#
#**                 Microsoft RPC Examples                  **#
#**                   xmit Application                      **#
#**        Copyright 1992 - 1999 Microsoft Corporation      **#
#**                                                         **#
#*************************************************************#

!include <Win32.Mak>

!if "$(CPU)" == "i386"
cflags = $(cflags) -D_CRTAPI1=_cdecl -D_CRTAPI2=_cdecl
!else
cflags = $(cflags) -D_CRTAPI1= -D_CRTAPI2=
!endif

all : xmitc xmits

# Make the client
xmitc : xmitc.exe
xmitc.exe : xmitc.obj xmitu.obj xmit_c.obj
    $(link) $(linkdebug) $(conflags) -out:xmitc.exe -map:xmitc.map \
      xmitc.obj xmitu.obj xmit_c.obj \
      rpcrt4.lib $(conlibsdll)

# xmit client main program
xmitc.obj : xmitc.c xmit.h xmitu.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

xmitu.obj : xmitu.c xmit.h xmitu.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# xmit client stub
xmit_c.obj : xmit_c.c xmit.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c


# Make the server executable
xmits : xmits.exe
xmits.exe : xmits.obj xmitu.obj xmitp.obj xmit_s.obj
    $(link) $(linkdebug) $(conflags) -out:xmits.exe -map:xmits.map \
      xmits.obj xmitu.obj xmit_s.obj xmitp.obj \
      rpcrt4.lib $(conlibsdll)

# xmit server main
xmits.obj : xmits.c xmit.h xmitu.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# remote procedures
xmitp.obj : xmitp.c xmit.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# xmit server stub file
xmit_s.obj : xmit_s.c xmit.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# Stubs and header file from the IDL file
xmit.h xmit_c.c xmit_s.c : xmit.idl xmit.acf
    midl $(MIDL_OPTIMIZATION) -oldnames -no_cpp xmit.idl

# Clean up everything
cleanall : clean
    -del *.exe

# Clean up everything but the .EXEs
clean :
    -del *.obj
    -del *.map
    -del xmit_c.c
    -del xmit_s.c
    -del xmit.h
