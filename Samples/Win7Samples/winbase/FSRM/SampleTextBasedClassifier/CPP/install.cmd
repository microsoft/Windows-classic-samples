@REM v1.0

@setlocal

set TARGET_SERVICE_NAME=FsrmSampleClassifier
set TARGET_DIR="%systemdrive%\FsrmSampleClassifier"


::
:: Create target directory
:: 
rd /s/q %TARGET_DIR%
md %TARGET_DIR%
if errorlevel 1 @goto :ERROR

::
:: Copy the DLLs
:: 
copy "%~dp0\debug\FsrmSampleClassificationModule.dll" %TARGET_DIR%
if errorlevel 1 @goto :ERROR

::
:: Register the DLL
:: 
%windir%\system32\regsvr32 /s %target_dir%\FsrmSampleClassificationModule.dll
if errorlevel 1 @goto :ERROR

::
:: Register with COM+
:: 
cscript "%~dp0\register_app.vbs" -register %TARGET_SERVICE_NAME% %target_dir%\FsrmSampleClassificationModule.dll "Sample Classifier" 
if errorlevel 1 @goto :ERROR

::
:: Make the module visible to FSRM
:: 
cscript "%~dp0\RegisterWithFsrm.vbs"
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


