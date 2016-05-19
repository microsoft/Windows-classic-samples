@REM v1.0

@setlocal

copy "bin\Debug\ManagedContentClassifier.dll"

set TARGET_DIR="%systemdrive%\ManagedContentClassifier"
set TARGET_COMPONENT_PROGID=Microsoft.Samples.Fsrm.ManagedContentClassifier.RegexClassifier


::
:: Create target directory
:: 
rd /s/q %TARGET_DIR%
md %TARGET_DIR%
if errorlevel 1 @goto :ERROR

::
:: Copy the DLLs
:: 
copy ManagedContentClassifier.dll %TARGET_DIR%
if errorlevel 1 @goto :ERROR

::
:: strong name
:: 
Sn -Vr %target_dir%\ManagedContentClassifier.dll
if errorlevel 1 @goto :ERROR

::
:: gacutil
:: 
gacutil /i %target_dir%\ManagedContentClassifier.dll
if errorlevel 1 @goto :ERROR


::
:: Make the module visible to FSRM
:: 
cscript "%~dp0\RegisterWithFsrmAsExternal.vbs"
if errorlevel 1 @goto :ERROR

reg import classreg.reg
reg import interfacereg.reg
reg import classreg2.reg


::
:: Register with COM+
:: 
cscript "%~dp0\register_fsrm_module.vbs" -register managedsampleclassifier %TARGET_COMPONENT_PROGID% 
if errorlevel 1 @goto :ERROR


@echo.
@echo Installation succeeded! 
@echo.
@goto :EOF



:ERROR
@echo.
@echo Installation FAILED! Error code: %errorlevel%
@echo.
@goto :EOF


