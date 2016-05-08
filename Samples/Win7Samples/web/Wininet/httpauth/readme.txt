Generic Win32-based GUI application

The RMCUIAPP sample is a generic Win32-based GUI application that acts as
a simple text editor.  The user will be allowed to type text and that is
about the extent of user functionality.  If the application is terminated
by a process using Restart Manager APIs it will save the data when RmShutdown 
is issued and reload data when the app is restarted after RmRestart is called.

This application showcases the minimal level of functionality required to 
enable a GUI application to be compliant to platform shutdown and 
restart by Restart Manager aware applications as well as by the Windows 
Error Reporting Service.

This application also provides a simple model of data serialization and 
restoration.  You can observe how you could model state recovery into your
own application.

The files included in this sample are:

RMGUIAPP.CPP - The .CPP source file

Build Instructions:

To build this sample application type nmake from the sample directory.

You must be in the SDK Vista build environment window.