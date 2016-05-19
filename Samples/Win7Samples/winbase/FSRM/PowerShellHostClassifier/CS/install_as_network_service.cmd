@REM v1.0

@setlocal
copy "bin\Debug\PowerShellHostClassifier.dll" .

set TARGET_DIR="%systemdrive%\PowerShellHostClassifier"
set TARGET_COMPONENT_PROGID=Microsoft.Samples.Fsrm.PowerShellHostClassifier.HostingClassifier


mkdir TARGET_DIR

::
:: Copy the DLLs
:: 
copy PowerShellHostClassifier.dll %TARGET_DIR%
if errorlevel 1 @goto :ERROR

::
:: strong name
:: 
Sn -Vr "%~dp0PowerShellHostClassifier.dll"
if errorlevel 1 @goto :ERROR

::
:: gacutil
:: 
gacutil /i "%~dp0PowerShellHostClassifier.dll"
if errorlevel 1 @goto :ERROR


::
:: Make the module visible to FSRM
:: 
cscript "%~dp0RegisterWithFsrmAsNetworkService.vbs"
if errorlevel 1 @goto :ERROR

reg import classreg2.reg
reg import eventlog.reg


@echo.
@echo Installation succeeded! 
@echo.
@goto :EOF



:ERROR
@echo.
@echo Installation FAILED! Error code: %errorlevel%
@echo.
@goto :EOF


