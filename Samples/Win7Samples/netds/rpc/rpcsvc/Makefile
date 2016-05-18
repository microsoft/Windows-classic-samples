#*************************************************************#
#**                                                         **#
#**                 Microsoft RPC Examples                  **#
#**                   Rpc NT Service                        **#
#**          Copyright 1992 - 1999 Microsoft Corporation    **#
#**                                                         **#
#*************************************************************#

!include <Win32.Mak>

all : client server

cflags= $(cflags) -GS

!if "$(CPU)" == "i386"
link = $(link) -SAFESEH
!endif

.c.obj:
    $(cc) $(cdebug:Od=Ox) -nologo -I. $(cflags) $(cvarsdll) $*.c

# For better performance replace $(cdebug) with $(cdebug:Od=Ox)
# Add -G4 -Oy on X86.

# Make the client side application
client : svcclnt.exe
svcclnt.exe : client.obj rpcsvc_c.obj
    $(link) $(linkdebug) $(conflags) -out:$@ \
      $** \
      rpcrt4.lib $(conlibsdll)

client.c : rpcsvc_c.c

# Make the server side application
server : rpcsvc.exe
rpcsvc.exe : server.obj service.obj rpcsvc_s.obj
    $(link) $(linkdebug) $(conflags) -out:$@ \
      $** \
      rpcrt4.lib $(conlibsdll)

server.c  : rpcsvc_s.c service.h
service.c : service.h

# Generated files depend on the .IDL and .ACF

rpcsvc_c.c : rpcsvc.idl rpcsvc.acf
    midl $(MIDL_OPTIMIZATION) -ms_ext -server none -cpp_cmd $(cc) -cpp_opt "-nologo -E"   rpcsvc.idl

# See the .ACF for a explanation why the -DSERVER flag here.

rpcsvc_s.c : rpcsvc.idl rpcsvc.acf
    midl $(MIDL_OPTIMIZATION) -ms_ext -client none -cpp_cmd $(cc) -cpp_opt "-nologo -E -DSERVER" rpcsvc.idl

# Clean up everything
cleanall : clean
    -del *.exe

# Clean up everything but the .EXEs
clean :
    -del *.obj
    -del *.map
    -del rpcsvc_c.c
    -del rpcsvc_s.c
    -del rpcsvc.h
