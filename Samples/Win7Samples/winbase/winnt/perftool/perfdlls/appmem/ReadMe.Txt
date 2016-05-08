APPMEM

Application Heap usage counters

These DLL's allow an application developer to instrument the heap
usage of an application program as determined by the calls to
GlobalAlloc, GlobalReAlloc and GlobalFree. The frequency of calls and
amount of memory used by these calls can be tracked via the
performance registry and Perfmon.


Installation

Once the project has been built, the performance counters can be
installed by following the steps below. The files will be found in
the pub subdirectory with the .dlls in the pub\<proc> dir.

    copy the appropriate dll's to the %systemroot%\system32 directory
    run: regedit appmem.reg
    run: lodctr appmem.ini

Use:

To have your application track memory usage, it needs to call the
following functions instead of the standard ones. These can be
conditionally compiled (or not) if you don't wish to use them all the
time.

        Std. Function       Tracking Function
        ----------------    ---------------------
        GlobalAlloc         GlobalAllocP
        GlobalReAlloc       GlobalReAllocP
        GlobalFree          GlobalFreeP

The .EXE will need to link with the APPMEMA.LIB in order to find
these new functions.

Once successfully compiled and linked, start PERFMON, then start the
executable program. In perfmon, select the "ADD to..." dialog and add
the program's instance of the desired counters to the display and
begin tracking the heap usage of the application.
