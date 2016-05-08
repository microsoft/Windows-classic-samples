UCS-Managed - Usermode Counter Sample for managed code providers.
=================================================
Demonstrates providing performance counter data from a .Net application using System.Diagnostics.PerformanceData namespace.
The functionality of this sample is identical to that of the native code UCS sample.

Languages
===============================
     This sample is available in the following language implementations:
     C#
     
Files
=================================================
Readme.txt:     This file

ucs.cs:         Usermode Counter Sample. Main implementation file.
				This could be a target application to be monitored.

ucs.man:        Counter manifest file. This file describes counter information (e.g., counter name,
                type, description).  You can use ecmangen.exe tool to help generate a counter manifest file.

ucs.rc:         Generated resource file that contains all localized strings. As ucs.man only contains 
                English strings, so does this file (and localization team can localize strings there).
                This file is generated from ucs.man by running ctrpp.exe tool. Example command-line:               
                ctrpp -rc ucs.rc ucs.man
                
ucs.res:        Compiled resource file generated from ucs.rc by running rc.exe tool. Example command-line:
				rc.exe /r /i "c:\Program Files\Microsoft SDKs\Windows\v7.0\Include" ucs.rc

 
Prerequisites
=================================================
This sample requires .NET 3.5 or later and Windows Vista or later.


To build the sample using Visual Studio 2008:
================================================
     1. Open Windows Explorer and navigate to the  directory.
     
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.
     
     NOTE that the compiled resource file, ucs.res, has been already added to your project's Application property page.      
     That causes Win32 resources describing the counter data to be linked with your managed assembly.
     
     ALSO NOTE the post-build step:  lodctr.exe /m:"$(TargetDir)ucs.man"
     Lodctr.exe tool registers counters on your computer. You need to run this tool as part of your application deployment.


To run the sample
=================================================           
     1. Make sure that the post-build lodctr.exe tool has executed successfully and that ucs.man is located in the same directory as ucs-managed.exe.
     
     2. Navigate to the directory that contains the ucs-managed.exe, using the command prompt or Windows Explorer.
     
     3. Type ucs-managed.exe at the command line, or double-click the icon for ucs-managed.exe to launch it from Windows Explorer.


To Test the sample
=================================================
    1. Open Perfmon.

        Start -> Run -> Type "Perfmon" -> ENTER

    2. Open the "Add counters" dialog. 
    
        You can see two new objects, "Geometric Waves Managed" and "Trigonometric Waves Managed", under the object

    3. Add a counter. 

        Choose, for instance, "Geometric Waves Managed" and its counter, "Square Wave" and select <All instances>
        in the "Instances of selected object" window, and then press ADD. 


To uninstall the sample
=================================================
    If you would like to uninstall the counter, run the following command:

        unlodctr /m:ucs.man
