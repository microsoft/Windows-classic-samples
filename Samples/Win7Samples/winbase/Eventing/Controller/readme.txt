=========================
EVENT CONTROL API SAMPLE
=========================


DESCRIPTION
====================================
TraceLog is an ETW trace collection and control utility. It uses the Event Tracing API to control (e.g. start, stop) logging sessions. 
The basic functions of Tracelog are:

1. Starting, stopping, updating and querying a trace session

2. Setting up a buffer to which the event traces are to be logged by the provider

3. Creating a log file to which the buffer is flushed

4. Providing more specific control over system level tracing

5. Controlling the level of tracing required


SAMPLE LANGUAGE IMPLEMENTATIONS
===============================
     This sample is available in the following language implementations:
     C


FILES
===============================
tracelog.c
     Controls eventing sessions


BUILD
===============================

To build the sample using msbuild:
=============================================
1.  Open a Command Prompt window and navigate to the following directory:
        Samples\WinBase\Eventing\Controller
2. Type "msbuild tracelog.sln".

To build the sample using Visual Studio:
================================================
1.  Open Windows Explorer and navigate to the following directory:
        Samples\WinBase\Eventing\Controller
2. Double-click the icon for the tracelog.sln solution file to open the file in Visual Studio.
3. In the Build menu, select Build Solution. 

To build the sample using the makefile:
================================================
1.  Open a Command Prompt window and navigate to the following directory:
        Samples\WinBase\Eventing\Controller
2. Type "nmake" to build the sample.

RUN
===============================

To run the sample using defaults:
==================================
1. From an elevated CMD prompt, navigate to the following directory:
        Samples\WinBase\Eventing\Controller\Output
2. Start the NT Kernel Logger by using the -start action.  By default the events are placed in the c:\LogFile.etl file.
        tracelog -start
3. To stop the session, use the –stop action:
        tracelog -stop
4. After starting and stopping the logging session, an ETL file should be created at c:\LogFile.etl.  This file stores events between the time the tracelog -start and tracelog –stop commands were issued.  The log file will contain information on processes, threads, disk, and network I/O.


To run the sample using other parameters:
===========================================
Consult the following Usage and Examples:

Usage
--------

tracelog [actions] [options] | [-h | -help | -?]

    actions:
        -start   [LoggerName] Starts up the [LoggerName] trace session
        -stop    [LoggerName] Stops the [LoggerName] trace session
        -update  [LoggerName] Updates the [LoggerName] trace session
        -enable  [LoggerName] Enables providers for the [LoggerName] session
        -disable [LoggerName] Disables providers for the [LoggerName] session
        -query   [LoggerName] Query status of [LoggerName] trace session
        -list                 List all trace sessions

    options:
        -um                   Use Process Private tracing
        -max <n>              Sets maximum buffers
        -f <name>             Log to file <name>
        -guid #<guid>         Provider GUID to enable/disable

        -h
        -help
        -?                    Display usage information


Examples
--------

1. tracelog -start <session_name> -guid <provider_guids> -f <logfile> 
   Starts tracing to file <logfile>.

2. tracelog -enable <session_name> -guid <provider_guids>
   Enables event providers specified in file <provider_guids> for the tracing session <session_name>.

3. tracelog -stop <session_name>
   Stops tracing of <session_name> session.

4. tracelog -update 
   If current logger is a realtime logger, this will switch the current logger to a non realtime, sequential logger.

5. tracelog -list
   Lists all active logging sessions.


