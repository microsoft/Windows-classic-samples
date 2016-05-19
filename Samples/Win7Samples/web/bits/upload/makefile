# +---------------------------------------------------------------------------
# 
#   Copyright (c) Microsoft Corporation. All rights reserved. 
# 
# 
#   BITS Upload sample
#   ==================
# 
#   Makefile
#
#   Please see the readme.htm file for more details about this application.
# 
# ----------------------------------------------------------------------------

!ifdef NTMAKEENV
!include $(NTMAKEENV)\makefile.def
!else

!IF "$(TARGETOS)" != "WINNT"
!ERROR  Sorry, BITS is not supported on non-NT platforms.
!ENDIF

#This app will run on Win2k and above, as long as BITS 1.5 is present in the system

#----- Include the PSDK's WIN32.MAK to pick up defines------------------------------------
!include <win32.mak>

#----- OUTDIR is defined in WIN32.MAK This is the name of the destination directory-------
all: \
	$(OUTDIR)\uploadsample.exe  \
	$(OUTDIR)\configure.js \
	$(OUTDIR)\newupload.asp \
	$(OUTDIR)\readme.htm  \
	displaymessage

LINK_OBJS= \
	$(OUTDIR)\main.obj \
	$(OUTDIR)\util.obj \
	$(OUTDIR)\cdialog.obj \
	$(OUTDIR)\cpack.obj \
	$(OUTDIR)\cmonitor.obj

LINK_LIBS= \
	$(guilibs)   \
    shell32.lib  \
    comctl32.lib \
    shlwapi.lib  \
    strsafe.lib  \
    bits.lib
        
#--------------------- EXE ---------------------------------------------------------------
# Please refer to WIN32.MAK for the different Linker options WIN32.MAK provides.


# Build rule for EXE
$(OUTDIR)\uploadsample.exe:  $(OUTDIR) $(LINK_OBJS) $(OUTDIR)\dialog.res
    $(link) $(ldebug) $(guilflags) $(olelibsdll) /PDB:$(OUTDIR)\uploadsample.pdb -out:$(OUTDIR)\uploadsample.exe $(LINK_OBJS) $(OUTDIR)\dialog.res $(LINK_LIBS)

# Misc files to copy
$(OUTDIR)\configure.js: $(OUTDIR) .\configure.js
    copy .\configure.js $(OUTDIR)

$(OUTDIR)\newupload.asp: $(OUTDIR) .\newupload.asp
    copy .\newupload.asp $(OUTDIR)

$(OUTDIR)\readme.htm: $(OUTDIR) .\readme.htm
    copy .\readme.htm $(OUTDIR)

#----- If OUTDIR does not exist, then create directory
$(OUTDIR):
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)


#--------------------- Compiling C/CPP Files ---------------------------------------------
#Please refer to WIN32.MAK for the different Compiler options WIN32.MAK provides.

EXTRA_FLAGS = -DUNICODE -D_UNICODE -EHsc -WX 

# Generic rule for building ALL CPP files and placing their OBJ's in the OUTDIR
.cpp{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvarsdll) $(EXTRA_FLAGS) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

#--------------------- Compiling Resource Files ------------------------------------------
#Please refer to WIN32.MAK for the different Resource Compiler options WIN32.MAK provides.

# Build rule for resource file
$(OUTDIR)\dialog.res: .\dialog.rc $(OUTDIR) resource.h
    $(rc) $(rcflags) $(rcvars) /fo $(OUTDIR)\dialog.res .\dialog.rc

#--------------------- Clean Rule --------------------------------------------------------
# Rules for cleaning out those old files
clean:
        $(CLEANUP)

#--------------------- Message to the user  ----------------------------------------------
displaymessage:
        @echo.
	@echo *
	@echo * To use this sample app, please follow these steps:
	@echo *
	@echo *  1) Make sure you have BITS 1.5 runtime files installed in your system. 
	@echo *
	@echo *     BITS 1.5 is a system component on Windows Server 2003. There are
	@echo *     redistributable packages available for Windows XP and Windows 2000 as well.
	@echo *     Please see the readme.htm file for more information.
	@echo *
	@echo *  2) Configure your IIS server to enable BITS uploads 
	@echo *
	@echo *     Please make sure that BITS Extensions for IIS is installed.
	@echo *     You will also need to configure a virtual directory for upload.
	@echo *     The script configure.js will help you do that, if you want to use
	@echo *     a local server. Execute the command 'cscript configure.js' to see
	@echo *     the command line options.
	@echo *     For more information on how to configure your IIS server for 
	@echo *     BITS upload, please see the readme.htm file
	@echo *     or the BITS SDK documentation.
	@echo *
	@echo *  3) Run $(OUTDIR)\uploadsample.exe
	@echo *

!endif
