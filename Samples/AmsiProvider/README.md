---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: IAntimalwareProvider interface sample
urlFragment: iantimalwareprovider-sample
description: Demonstrates how to use the Antimalware Provider Interface to register as a provider and scan an incoming stream. 
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# IAntimalwareProvider interface sample

Demonstrates how to use the Antimalware Provider Interface to register as a provider and scan an incoming stream.

The sample implements the
[IAntimalwareProvider](https://msdn.microsoft.com/en-us/library/windows/desktop/dn889593(v=vs.85).aspx) interface
which receives a stream to be scanned in the form of an IAmsiStream interface.

The sample demonstrates a simple provider which computes the XOR of all the bytes in the file and logs the results. It reports all streams as safe.

Note that the provider is loaded as an in-process server, which means that you need to install both 32-bit and 64-bit versions in order to support both 32-bit and 64-bit applications.

## Instructions

### Building and installing the sample provider

1. Load the Project solution.
2. Build the Project.
3. From an elevated command prompt, go to the output directory and type `regsvr32 AmsiProvider.dll`.

If your system has other providers installed, they may take priority over the sample provider. To prevent this from happening (for testing purposes), go to the `HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\AMSI\Providers` and `HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\AMSI\Providers` registry keys and temporarily rename the other providers so that the system will use the sample provider.

### Exercising the sample provider

Events logged by the sample provider can be captured using ETW tools such as xperf. The log files are generated in ETL format so they can be viewed and processed by the Windows Performance Toolkit (WPT), as well as utilities such as *tracerpt.exe* or *xperf.exe*.

1. From an elevated command prompt, type `xperf.exe -start mySession -f myFile.etl -on 00604c86-2d25-46d6-b814-cd149bfdf0b3` to begin capturing events from the provider used by the sample.
2. From an unelevated command prompt, launch PowerShell with the Bypass execution policy.
The PowerShell program should be the same bitness as the project you built and installed.
   * To run 32-bit PowerShell on a 32-bit system, or 64-bit PowerShell on a 64-bit system: `powershell -ep bypass`
   * To run 32-bit PowerShell on a 64-bit system: `C:\Windows\SysWOW64\WindowsPowerShell\v1.0\powershell -ep bypass`
3. Run some PowerShell commands. For example, type `calc` to launch calc.
4. Exit PowerShell.
5. From an elevated command prompt, type `xperf.exe -stop mySession` to stop capturing events.
6. View the `myFile.etl` trace graphically in WPA, or generate a text version by typing `tracerpt myFile.etl`.

### Uninstalling the sample provider

1. From an elevated command prompt, go to the output directory and type `regsvr32 /u AmsiProvider.dll`.
2. If you temporarily renamed conflicting providers when you installed the sample provider, rename the keys back.
