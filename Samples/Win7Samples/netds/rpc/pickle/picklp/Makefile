#*************************************************************#
#**                                                         **#
#**                 Microsoft RPC Examples                  **#
#**                   pickle Application                    **#
#**          Copyright 1992 - 1999 Microsoft Corporation    **#
#**                                                         **#
#*************************************************************#

!include <Win32.Mak>

!if "$(CPU)" == "i386"
cflags = $(cflags:G3=Gz)
!else
cflags = $(cflags) /W3 -D_CRTAPI1= -D_CRTAPI2=
!endif

!if "$(CPU)" == "i386"
cflags = $(cflags) -D_CRTAPI1=_cdecl -D_CRTAPI2=_cdecl
!endif

cflags = $(cflags) -GS

!if "$(CPU)" == "i386"
link = $(link) -SAFESEH
!endif

all : picklpc

# Make the picklp client
picklpc : picklpc.exe
picklpc.exe : picklpc.obj picklp_c.obj
    $(link) $(linkdebug) $(conflags) -out:picklpc.exe \
      picklpc.obj picklp_c.obj \
      rpcrt4.lib $(conlibsdll)

# client main program
picklpc.obj : picklpc.c picklp.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) $*.c

# client stub
picklp_c.obj : picklp_c.c picklp.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) $*.c


# Stub and header file from the IDL file
picklp.h picklp_c.c: picklp.idl picklp.acf
    midl $(MIDL_OPTIMIZATION) -oldnames -no_cpp picklp.idl

# Clean up everything
cleanall : clean
    -del *.exe

# Clean up everything but the .EXEs
clean :
    -del *.obj
    -del *.map
    -del picklp_c.c
    -del picklp.h

