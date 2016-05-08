@echo off

echo - copying files to %systemdrive%\Eventing\Advanced\CSharp

md %systemdrive%\Eventing\Advanced\CSharp >NUL 2>&1
copy output\AdvancedProvider.exe %systemdrive%\Eventing\Advanced\CSharp
copy AdvancedProvider.man %systemdrive%\Eventing\Advanced\CSharp
pushd %systemdrive%\Eventing\Advanced\CSharp

echo - install provider using wevtutil (inbox tool)

wevtutil im AdvancedProvider.man

echo - start logging session for AdvancedProvider.exe using logman (inbox tool)

logman start -ets AdvancedProvider -p "Microsoft-Windows-SDKSample-AdvancedProvider-CS" 0 0 -o AdvancedProvider.etl

pause 

echo - Execute provider

AdvancedProvider.exe

pause

echo - Stop provider session 

logman stop AdvancedProvider -ets

pause 

echo - generate dumpfile using tracerpt (inbox tool).

tracerpt -y AdvancedProvider.etl 

pause

echo - Uninstall provider

wevtutil um AdvancedProvider.man

pause

echo - open dumpfile

notepad dumpfile.xml
popd

echo - open event viewer and verify some events for this provider have been logged.
