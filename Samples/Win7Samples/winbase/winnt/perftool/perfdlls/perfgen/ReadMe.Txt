Using the Signal Generator Performance DLL

Overview:

The Signal Generator performance DLL provides 3 waveforms at 5
different periods for use with the Performance Monitor to provide
data at a known rate and value. This can be useful in testing
applications that use performance data and need predictable values to
test against.


Building and Installing:

The DLL must be built from the source code using the Windows NT build
Utilities and then installed on the target system using the following
steps:

    1. Copy the PerfGen.DLL that was built to the %systemroot%\system32 
    directory.

    2. load the driver entries into the registry using the following
    command line:

        REGEDIT PERFGEN.REG

    3. load the performance names into the registry using the command
    line:

        LODCTR PERFGEN.INI

At this point all the software is installed and it ready to use.
Start Perfmon and select the "Signal Generator" object to select the
desired wave form and period.

NOTE: The system may need to be restarted after these instructions 
are completed for this object to be seen by remote computers.
