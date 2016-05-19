 Copyright (c) Microsoft Corporation. All rights reserved.
Windows Installer utility scripts for use with Windows Scripting Host

WiLstPrd.vbs - lists products, product properties, features, and components
WiImport.vbs - imports files to an MSI database
WiExport.vbs - exports files from an MSI database
WiSubStg.vbs - adds a transform or nested MSI as a substorage
WiStream.vbs - adds an external stream to a MSI package (e.g CAB file)
WiMerge.vbs  - merges two MSI databases
WiGenXfm.vbs - generates a transform from two databases, or compares them
WiUseXfm.vbs - applies a transform to an MSI database
WiLstXfm.vbs - lists an MSI transform file (CSCRIPT only)
WiDiffDb.vbs - lists the differences between two databases (CSCRIPT only)   
WiLstScr.vbs - lists an installer script file (CSCRIPT only)
WiSumInf.vbs - displays and updates summary information stream
WiPolicy.vbs - manages installer policy settings
WiLangId.vbs - reports and updates the language and codepage of a package
WiToAnsi.vbs - copies a Unicode text file to the same or a new Ansi text file
WiFilVer.vbs - updates File table sizes and versions from source file tree
WiMakCab.vbs - generates compressed file cabinets and updates package
WiRunSQL.vbs - executes SQL statements against an installer database
WiTextIn.vbs - copies an ANSI text file into a database string column value
WiCompon.vbs - lists components and their composition in an installer database
WiFeatur.vbs - lists features and their composition in an installer database
WiDialog.vbs - previews all or specified dialogs in an installer database

All scripts display help screens describing their command line arguments,
either if the first argument contains a ?, or if too few arguments are given.
Scripts return a value of 0 for success, 1 if help invoked, or 2 if failure.

Windows Scripting Host is actually two hosts, use //? to obtain help:
Cscript.exe displays to standard out - useful for batch files and build scripts
Wscript.exe displays the output in a window - the default for extension .VBS
When run from a Win NT DOS box, only the base name needs to be specified, and
if the tools are on the PATH, then they will be found like EXE or BAT files.
To redirect output to a file, you must use Cscript {scriptname}.VBS > {file}.

To download and install Windows Scripting Host (not required for Windows2000):
  http://msdn.microsoft.com/scripting/windowshost/download/default.htm
To download and update Windows scripting engines:
  http://msdn.microsoft.com/scripting/vbscript/download/vbsdown.htm
or
  http://www.microsoft.com/msdownload/vbscript/scripting.asp

The utility makecab.exe is located in the Windows Installer SDK
at \Patching\makecab.exe.

For more information about these samples, see the section
"Windows Installer Scripting Examples" in the Windows Installer help files.
