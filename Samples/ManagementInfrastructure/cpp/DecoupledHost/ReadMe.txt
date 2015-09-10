====================================================================================
DECOUPLED HOST SAMPLE
====================================================================================

The current directory contains source code demonstrating how to create an application
to host decoupled provider using MIAPI.


NOTE: Please see ..\ReadMe.txt (Section 2. PREPARATION) for instructions on how to 
create environment variables which will be used in this sample.

====================================================================================
INSTRUCTIONS
====================================================================================

1. Compile

    Open %MISAMPLEDIR%\misample.sln file with visual studio 2012 and
    make sure following settings are correct, then you will be able to build.

        - Click menu Build->Configuration Manager, select desired Active solution platform,
          "x64" for example.
        - Right Click "dcuphost" project and select properties.
        - Goto 'Configuration Properties' -> Linker -> Input -> Additional Dependencies
          Make sure "mi.lib;%(AdditionalDependencies)" was added there.

    NOTE: If the target windows 8 is 32bit (x86), then you need to set Active solution platform as Win32.


2. Run

   Once the project above is built, you will have created a console application
   called dcuphost.exe.
   To test the decoupled host application, you also need a provider (to be hosted)
   registered as decoupled provider. So, copy both dcuphost.exe and process.dll to a
   Windows 8 or Windows Server 2012 machine (Assume you copied them to c:\temp).

   1) Register the process provider as decoupled provider by running following commandline from elevated commandline prompt (cmd.exe):
   Register-CimProvider.exe -Namespace Root/StandardCimV2/dcupsample -ProviderName process -Path C:\temp\provider.dll -Decoupled O:BAG:BAD:(A;;0x1;;;BA)(A;;0x1;;;NS) -Impersonation True

   2) Invoke the following command line from elevated commandline prompt (cmd.exe):
   DcupHost.exe -Namespace root/standardcimv2/dcupsample -ProviderName process -ProviderPath c:\temp\provider.dll

   3) Read ..\Process\Readme.txt to find how to consume the data from process provider.
   All client application/cmdlets will work against decoupled provider as well.

