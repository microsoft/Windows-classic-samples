Standard DLL & Example of Creating a Custom Control Class


SUMMARY
=======

The SPINCUBE sample provides a generic Win32 DLL template. It demonstrates 
the use of DLL entry points, exported variables, and using the C run-time in 
a DLL. This sample also provides a functional example of how to create a 
custom control class that can be used by applications, such as SPINTEST.EXE, 
as well as the Dialog Editor.

MORE INFORMATION
================

SPINCUBE.DLL contains the control window procedure and the interface 
functions required by the Dialog Editor (see SPINCUBE.C), as well as the 
control paint routines (see PAINT.C). SPINTEST.EXE is a small test program 
that loads SPINCUBE.DLL and creates a few of the custom controls.

To test SPINCUBE with the Dialog Editor:

 1. Start the editor. From the File menu, choose Open Custom.
 2. Enter the path and filename of SPINCUBE.DLL.
 3. Create a new dialog box and choose a custom control button from the 
    control palette (lower-right corner).
 4. Click the dialog box to create a SPINCUBE control.
 5. Save the dialog box template.
 6. Inspect the .DLG file that was created.

