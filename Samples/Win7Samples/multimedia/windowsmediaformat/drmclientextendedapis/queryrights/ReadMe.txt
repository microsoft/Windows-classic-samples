========================================================================
       WIN32 Console Application : QueryRights 
========================================================================

Demonstrates how to make optimized simple rights queries and license 
state qeuries using the Windows Media DRM Client Extended APIs.

USAGE:

QueryRights <KIDFile> <ReportFile>

    KIDFile    = Name of the file containing a list of Key ID values to 
                 query against. See the file format information below.
    ReportFile = Name of the outpout report file. If this file exists, 
                 it will be overwritten.

HOW TO BUILD:

  In order to build the sample executable, open the project file 
QueryRights.sln in Visual C++ and build the project.


REMARKS:

The input file must be a text file with the following format:

KIDFILE
n
<KIDString1>
...


Where n is the number of KIDStrings in the file.



IMPORTANT INTERFACES DEMONSTRATED IN THIS SAMPLE:

    IWMDRMLicenseQuery