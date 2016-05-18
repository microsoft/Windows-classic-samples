Simple Service


SUMMARY
=======

The SERVICE sample demonstrates how to create and install a service.

In this particular sample, the service merely opens a named pipe (the name 
defaults to \\.\pipe\simple) and waits for read and write operations to the 
pipe. If the pipe receives input, it creates the string:

    Hello! [<input goes here>]

and sends it back to the client through the pipe.

The service can be started and stopped from the control panel Services 
applet, the net command, or by using the service controller utility (see 
MORE INFORMATION).

The service also provides command-line parameters which install, remove, or 
run (debug) the service as a console application.

MORE INFORMATION
================

To aid in writing and debugging services, the SDK contains a utility
(MSTOOLS\BIN\SC.EXE) that can be used to control, configure, or obtain 
service status. SC displays complete status for any service in the service 
database, and allows any of the configuration parameters to be easily 
changed at the command line. For more information on SC.EXE, type SC at the 
command line.

Usage:

To install the service, first compile everything, and then type:

    simple -install

Now, let's look at SC's command-line parameters:

    sc

To start the service, use the "net start" command, the control panel 
Services applet, or the command:

    sc start simpleservice

Verify that the service has entered the RUNNING state:

    sc query simpleservice

Once the service has been started, you can use the CLIENT program to verify 
that it really is working, using the syntax:

    client 

which should return the response:

    Hello! [World]

If, after playing with the sample, you wish to remove the service, simply 
say:

    simple -remove

You may change the name of the pipe by specifying -pipe <pipename> as a 
startup parameter for both CLIENT and SIMPLE. The string passed in by CLIENT 
can be changed by specifying -string <string>.

Notes:

1) The use of the SERVICE.H header file and the accompanying SERVICE.C file 
simplifies the process of writing a service. You as a developer simply need 
to follow the TODO's outlined in the header file, and implement the 
ServiceStart and ServiceStop functions for your service.

There is no need to modify the code in SERVICE.C. Just add SERVICE.C to your 
project and link with the following libraries:

    libcmt.lib 
    kernel32.lib 
    advapi.lib 
    shell32.lib

2) Install/Remove functionality should not be included in a production service.  
The functionality is included in this service for illustration purposes, and as 
a convenience to developers.

 

