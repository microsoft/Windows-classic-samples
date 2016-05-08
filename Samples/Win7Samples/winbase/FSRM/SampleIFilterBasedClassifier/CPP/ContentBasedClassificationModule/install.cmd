@REM v1.0

@setlocal

set TARGET_SERVICE_NAME=FsrmSampleIFilterClassifier
set TARGET_DIR="%systemdrive%\FsrmIFilterClassifier"


::
:: Create target directory
:: 
rd /s/q %TARGET_DIR%
md %TARGET_DIR%
if errorlevel 1 @goto :ERROR

::
:: Copy the DLLs
:: 
copy "%~dp0\x64\debug\ContentBasedClassificationModule.dll" %TARGET_DIR%
if errorlevel 1 @goto :ERROR

copy "%~dp0\..\FsrmTextReader\x64\debug\FsrmTextReader.dll" %TARGET_DIR%
if errorlevel 1 @goto :ERROR

::
:: Register the DLL
:: 
%windir%\system32\regsvr32 /s %target_dir%\ContentBasedClassificationModule.dll
if errorlevel 1 @goto :ERROR

%windir%\system32\regsvr32 /s %target_dir%\FsrmTextReader.dll
if errorlevel 1 @goto :ERROR

::
:: Register with COM+
:: 
cscript "%~dp0\register_app.vbs" -register %TARGET_SERVICE_NAME% %target_dir%\ContentBasedClassificationModule.dll "Sample IFilter Based Classifier" 
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


