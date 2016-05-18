=================================================
GLOBAL AGGREGATE PERFORMANCE COUNTER SAMPLE
=================================================
   

DESCRIPTION
===============================
This sample demonstrates the following performance counter concepts:
    1. Defining the metadata for global aggregate performance counters.
    2. Registration of performance counters.
    3. Providing data for global aggregate performance counters.


SAMPLE LANGUAGE IMPLEMENTATIONS
=================================================
    This sample is available in the following language implementations:
        C


FILES
=================================================
gas.c
        Global Aggregate Sample code. This can be considered the target application to be monitored.

gas.man
        Counter manifest file. This file defines performance counter metadata.

gas.rc
        Resource file that contains all localized strings. Note that 
.man files only contain English strings; .rc files are used to localize the strings defined in .man files. 


BUILD
===============================

To build the samples using msbuild:
=============================================
1. Open an elevated CMD prompt window and navigate to the following directory:
        Samples\WinBase\PerfCounters\GlobalAggregate\CPP
2. Type “msbuild gas.sln”.

To build the sample using Visual Studio:
=================================================
1.  Open Windows Explorer and navigate to the following directory:
        Samples\WinBase\PerfCounters\GlobalAggregate\CPP 
2. Double-click the icon for the gas.sln solution file to open the file in Visual Studio.
3. In the Build menu, select Build Solution. 

To build the sample using the makefile:
=================================================
1.  Open an elevated CMD prompt window and navigate to the following directory:
        Samples\WinBase\PerfCounters\GlobalAggregate\CPP 
2. Type "nmake" to build the sample.


RUN
===============================

To run the provider:
=================================================
1. Open an elevated CMD prompt window
2. Put gas.man and gas.exe in the same directory.
3. Use the following command to register the counter manifest with the system:
        lodctr /m:gas.man
4. Run the sample provider:
        gas.exe

To consume data from the provider:
=================================================
1. Type "perfmon" at a CMD prompt or Run dialog to start the PerfMon tool.
2. Select "Performance Monitor" under "Monitoring Tools".
2. Click the "Add counters" button (green '+' sign). 
3. Select "Aggregated Trigonometric Waves" and expand to see counters provided by gas.exe.
4. Select any of the counters listed (e.g. "Sine Wave") and click "Add".
5. Click "OK".  

To uninstall the sample
=================================================
1. Open an elevated CMD prompt window.
2. Use the following command to uninstall the counter manifest from the system. 
        unlodctr /m:gas.man
