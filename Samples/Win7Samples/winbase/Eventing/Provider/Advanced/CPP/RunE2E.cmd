@echo off

echo - copying files to %systemdrive%\Eventing\Advanced\CPP

md %systemdrive%\Eventing\Advanced\CPP >NUL 2>&1
copy Debug\AdvancedProvider.exe %systemdrive%\Eventing\Advanced\CPP
copy AdvancedProvider.man %systemdrive%\Eventing\Advanced\CPP
pushd %systemdrive%\Eventing\Advanced\CPP

echo - Install the provider using Windows commandline tool wevtutil.exe

wevtutil im AdvancedProvider.man

echo - Start the logging session for AdvancedProvider.exe using Windows commandline tool logman.exe

logman start -ets AdvancedProvider -p "Microsoft-Windows-SDKSample-AdvancedProvider" 0 0 -o AdvancedProvider.etl

pause 

echo - Execute the provider

AdvancedProvider.exe

pause

echo - Stop the provider session 

logman stop AdvancedProvider -ets

pause 

echo - Generate a dumpfile using Windows commandline tool tracerpt.exe

tracerpt -y AdvancedProvider.etl 

pause

echo - Uninstall the provider

wevtutil um AdvancedProvider.man

pause

echo - open dumpfile.xml

notepad dumpfile.xml
popd

echo - open event viewer and verify some events for this provider have been logged.
