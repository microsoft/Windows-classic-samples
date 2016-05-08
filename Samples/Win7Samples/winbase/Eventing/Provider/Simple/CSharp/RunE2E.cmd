@echo off

echo - copying files to %systemdrive%\Eventing\Simple\CSharp

md %systemdrive%\Eventing\Simple\CSharp >NUL 2>&1
copy output\SimpleProvider.exe %systemdrive%\Eventing\Simple\CSharp
copy SimpleProvider.man %systemdrive%\Eventing\Simple\CSharp
pushd %systemdrive%\Eventing\Simple\CSharp

echo - install provider using wevtutil (inbox tool)

wevtutil im SimpleProvider.man

echo - start logging session for SimpleProvider.exe using logman (inbox tool)

logman start -ets SimpleProvider -p "Microsoft-Windows-SDKSample-SimpleProvider-CS" 0 0 -o SimpleProvider.etl

pause 

echo - Execute provider

SimpleProvider.exe

pause

echo - Stop provider session 

logman stop SimpleProvider -ets

pause 

echo - generate dumpfile using tracerpt (inbox tool).

tracerpt -y SimpleProvider.etl 

pause

echo - Uninstall provider

wevtutil um SimpleProvider.man

pause

echo - open dumpfile

notepad dumpfile.xml
popd

echo - open event viewer and verify some events for this provider have been logged.
