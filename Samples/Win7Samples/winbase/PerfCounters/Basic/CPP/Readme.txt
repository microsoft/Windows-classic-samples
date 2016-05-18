=================================================
BASIC PERFORMANCE COUNTER SAMPLE
=================================================    


DESCRIPTION
===============================
This sample demonstrates the following performance counter concepts:
    1. Defining the metadata for simple performance counters
    2. Registration of performance counters.
    3. Providing performance counter data.


SAMPLE LANGUAGE IMPLEMENTATIONS
=================================================
    This sample is available in the following language implementations:
        C


FILES
================================================= 

ucs.c          
        Usermode Counter Sample code. This can be considered the target application to be monitored.

ucs.man
        Counter manifest file. This file defines performance counter metadata.

ucs.rc
        Resource file that contains all localized strings. Note that 
.man files only contain English strings; .rc files are used to localize the strings defined in .man files. 


BUILD
===============================

To build the samples using msbuild:
=============================================
1. Open an elevated CMD prompt window and navigate to the following directory:
        Samples\WinBase\PerfCounters\Basic\CPP
2. Type “msbuild ucs.sln”.

To build the sample using Visual Studio:
=================================================
1.  Open Windows Explorer and navigate to the following directory:
        Samples\WinBase\PerfCounters\Basic\CPP 
2. Double-click the icon for the ucs.sln solution file to open the file in Visual Studio.
3. In the Build menu, select Build Solution. 

To build the sample using the makefile:
=================================================
1.  Open an elevated CMD prompt window and navigate to the following directory:
        Samples\WinBase\PerfCounters\Basic\CPP
2. Type "nmake" to build the sample.


RUN
===============================

To run the provider:
=================================================
1. Open an elevated CMD prompt window
2. Put ucs.man and ucs.exe in the same directory.
3. Use the following command to register the counter manifest with the system:
        lodctr /m:ucs.man
4. Run the sample provider:
        ucs.exe

To consume data from the provider:
=================================================
1. Type "perfmon" at a CMD prompt or Run dialog to start the PerfMon tool.
2. Select "Performance Monitor" under "Monitoring Tools".
2. Click the "Add counters" button (green '+' sign). 
3. Select and expand "Geometric Waves" and "Trigonometric Waves" to see counters provided by ucs.exe.
4. Select any of the counters listed (e.g. "Square Wave")
5. Select "<All instances>" and click "Add".
5. Click "OK".  

To uninstall the sample
=================================================
1. Open an elevated CMD prompt window.
2. Use the following command to uninstall the counter manifest from the system. 
        unlodctr /m:ucs.man


To run the sample
=================================================
    1. Put ucs.man and ucs.exe in the same location, and if ucs.exe.mui is generated, put it in .\en-US.
       For example, if you copy ucs.exe and ucs.man to c:\ucs, then you should copy ucs.exe.mui to 
       c:\ucs\en-US .  
 
    2. Install a manifest: This step is for registering the couter information to the system.

        lodctr /m:ucs.man

    3. Run the sample provider

        type "ucs.exe"


To Test the sample
=================================================
    1. Open Perfmon.

        Start -> Run -> Type "Perfmon" -> ENTER

    2. Open the "Add counters" dialog. 
    
        You can see two new objects, "Geometric Waves" and "Trigonometric Waves", under the object

    3. Add a counter. 

        Choose, for instance, "Geometric Waves" and its counter, "Square Wave" and select <All instances>
        in the "Instances of selected object" window, and then press ADD. 


To uninstall the sample
=================================================
    If you would like to uninstall the counter, run the following command:

        unlodctr /m:ucs.man
