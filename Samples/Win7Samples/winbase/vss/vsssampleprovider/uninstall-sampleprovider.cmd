@rem @echo off
@setlocal


@rem the DLLs to be uninstalled must be either in the current directory as this script or
@rem in the SDK BIN directory.
@set FOUND_FILES=0
@set CMD_DIR=%~dp0


:uninstallfromCurrentDir
@set DLL_DIR=%CMD_DIR%
@set VBS_DIR=%CMD_DIR%
@call :checkfiles
@if %FOUND_FILES% EQU 0 (goto :uninstallfromSDKDirs) else (goto :startuninstall)


:uninstallfromSDKDirs
@set DLL_DIR=%CMD_DIR%\..\..\..\..\BIN
@set VBS_DIR=%CMD_DIR%
@call :checkfiles
@if %FOUND_FILES% EQU 0 (goto :missingfiles) else (goto :startuninstall)



:startuninstall

net stop vds
net stop vss
net stop swprv

reg.exe delete HKLM\SYSTEM\CurrentControlSet\Services\Eventlog\Application\VssSampleProvider /f

cscript "%VBS_DIR%\register_app.vbs" -unregister "VssSampleProvider"
regsvr32 /s /u "%DLL_DIR%\VssSampleProvider.dll"

echo.
goto :EOF


:checkfiles 

@if not exist "%DLL_DIR%\VssSampleProvider.dll" goto :EOF
@if not exist "%DLL_DIR%\VstorInterface.dll"    goto :EOF
@if not exist "%VBS_DIR%\register_app.vbs"      goto :EOF
@set FOUND_FILES=1
@goto :EOF


:missingfiles

@echo.
@echo One or more important files are missing. All the files listed below either need
@echo to be copied to a single directory (e.g. C:\vsssampleprovider), or stays in 
@echo their original directories in the SDK BIN and SAMPLES.
@echo.
@echo   VssSampleProvider.dll
@echo   VstorInterface.dll
@echo   register_app.vbs
@echo   install-sampleprovider.cmd
@echo   uninstall-sampleprovider.cmd
@echo.
@goto :EOF
