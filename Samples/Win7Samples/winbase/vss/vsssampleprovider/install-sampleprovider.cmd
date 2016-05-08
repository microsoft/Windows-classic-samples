@rem @echo off
@setlocal


@rem the DLLs to be installed must be either in the current directory as this script or
@rem in the SDK BIN directory.
@set FOUND_FILES=0
@set CMD_DIR=%~dp0


:installfromCurrentDir
@set DLL_DIR=%CMD_DIR%
@set VBS_DIR=%CMD_DIR%
@call :checkfiles
@if %FOUND_FILES% EQU 0 (goto :installfromSDKDirs) else (goto :goodproc)


:installfromSDKDirs
@set DLL_DIR=%CMD_DIR%\..\..\..\..\BIN
@set VBS_DIR=%CMD_DIR%
@call :checkfiles
@if %FOUND_FILES% EQU 0 (goto :missingfiles) else (goto :goodproc)



:goodproc

rem Remove existing installation
call "%CMD_DIR%\uninstall-sampleprovider.cmd"

@rem Get the complete %DLL_DIR% and %VBS_DIR%
@pushd %DLL_DIR%
@set DLL_DIR=%CD%
@popd
@pushd %VBS_DIR%
@set VBS_DIR=%CD%
@popd

rem Register VSS hardware provider
cscript "%VBS_DIR%\register_app.vbs" -register "VssSampleProvider" "%DLL_DIR%\VssSampleProvider.dll" "VSS HW Sample Provider"

set EVENT_LOG=HKLM\SYSTEM\CurrentControlSet\Services\Eventlog\Application\VssSampleProvider
reg.exe add %EVENT_LOG% /f
reg.exe add %EVENT_LOG% /f /v CustomSource /t REG_DWORD /d 1
reg.exe add %EVENT_LOG% /f /v EventMessageFile /t REG_EXPAND_SZ /d "%DLL_DIR%\VssSampleProvider.dll"
reg.exe add %EVENT_LOG% /f /v TypesSupported /t REG_DWORD /d 7

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
