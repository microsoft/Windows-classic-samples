@REM v1.0

@setlocal
:: Leave the powershell host classifier dll in TARGET_DIR=%systemdrive%\PowerShellHostClassifier

::
:: Make the module visible to FSRM
:: 
cscript %~dp0UnregisterFromFsrm.vbs
if errorlevel 1 @goto :ERROR

::
:: Unregister from COM+
:: 
cscript %~dp0\register_fsrm_module.vbs -unregister PowerShellHostClassifier 
if errorlevel 1 @goto :ERROR


@echo.
@echo Uninstall succeeded! 
@echo.
@goto :EOF



:ERROR
@echo.
@echo Uninstall FAILED! Error code: %errorlevel%
@echo.
@goto :EOF


