@echo off

echo - copying files to %systemdrive%\Eventing\Simple\CPP

md %systemdrive%\Eventing\Simple\CPP >NUL 2>&1
copy Debug\SimpleProvider.exe %systemdrive%\Eventing\Simple\CPP
copy SimpleProvider.man %systemdrive%\Eventing\Simple\CPP
pushd %systemdrive%\Eventing\Simple\CPP

echo - Install the provider using Windows commandline tool wevtutil.exe

wevtutil im SimpleProvider.man

echo - Start the logging session for SimpleProvider.exe using Windows commandline tool logman.exe

logman start -ets SimpleProvider -p "Microsoft-Windows-SDKSample-SimpleProvider" 0 0 -o SimpleProvider.etl

pause 

echo - Execute the provider

SimpleProvider.exe

pause

echo - Stop the provider session 

logman stop SimpleProvider -ets

pause 

echo - Generate a dumpfile using Windows commandline tool tracerpt.exe

tracerpt -y SimpleProvider.etl 

pause

echo - Uninstall the provider

wevtutil um SimpleProvider.man

pause

echo - open dumpfile.xml

notepad dumpfile.xml
popd

echo - open event viewer and verify some events for this provider have been logged.
