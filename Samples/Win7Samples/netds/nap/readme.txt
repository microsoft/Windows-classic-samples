Network Access protection Sample
================================
     Demonstrates NAP System health agent, NAP System Health Validator, and NAP Enforcement Client


Sample Language Implementations
===============================
     This sample is available in the following language implementations:
     C++


To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the sample directory.
     2. Type msbuild [Solution Filename].


To build the sample using Visual Studio 2008:
================================================================
     1. Open Windows Explorer and navigate to the sample directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     The application will be built in Win32\Debug or X64\Release directory. 


To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type [ExecutableFile] at the command line, or double-click the icon for [SampleExecutable] to launch it from Windows Explorer.


Additional Run Steps:
=====================
     1. Place SdkShv.dll inside %windir%\system32 and run regsvr32 /s SdkShv.dll on the NAP server.
     2. Run SdkSha.exe /? on the NAP client for further instructions.
     3. Run SdkQec.exe /? on the NAP client for further instructions.


To run SdkShaInfo.dll:
=====================
     1. Run SdkSha.exe in a command prompt on the NAP client
     2. In a separate command prompt on the NAP client, type regsvr32 /s SdkShaInfo.dll
     3. Run "netsh nap client show state" on the NAP client and note the parameters displayed for the SDK SHA sample differ from the parameters 
	displayed when SdkShaInfo.dll is not registered. The SHA\DLL\Messages.mc file can be edited to change the parameters displayed 
	for the SDK SHA sample. 


SHV configuration UI install/uninstall
=======================================
     1. Place SdkShv.dll inside %windir%\system32 and run regsvr32 /s SdkShv.dll on the NAP server
     2. Run sampleshvui.exe /regserver to install the sample SHV configuration UI on the NAP server
     3. Run sampleshvui.exe /unregserver to un-install the sample SHV configuration UI from the NAP server
	
