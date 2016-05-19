Printing


SUMMARY
=======

The PRINTER sample does the following:

  - Shows how to print using both the CreateDC and the PrinterDlg methods 
    for acquiring a printer HDC. The user is allowed to print different 
    graphical objects, as well as a complete device font set. An "Abort" 
    dialog box is also implemented.
  - Provides complete device capabilities for all printers and the display.
  - Provides information (levels 1 and 2) returned by a call to 
    EnumPrinters.
  - Shows how to enumerate fonts for a particular DC.
  - Illustrates differences between the various mapping modes.
  - Demonstrates GDI features.

MORE INFORMATION
================

The main application window contains a menu and a toolbar. The submenus are:

Print

Calls CreateDC to get a device context for the selected printer in the 
toolbar combo box, and then prints the current graphics options to this DC.

PrintDlg

Calls PrintDlg to retrieve a device context for a printer, then prints out 
current graphics options to this DC.

GetDeviceCaps

Retrieves device capabilities for devices currently selected in the toolbar 
combo box, and displays them in a dialog box.

EnumPrinters

Retrieves level 1 and 2 information returned by EnumPrinters and displays 
this information in a dialog box.

GetPrinterDriver

Returns level 1 and 2 information returned by GetPrinterDriver (for 
currently selected printer) and displays this information in a dialog box.

EnumPrinterDrivers

Returns level 1 and 2 information returned by EnumPrinterDrivers and 
displays this information in a dialog box.

Refresh

Refreshes the contents for the toolbar combo box.

About

Application information dialog

Mapping Modes

User selects between different mapping modes.

Graphics

User selects different primitives to display.

Pen

User can configure size, color, and style of drawing pen.

Brush

User can configure size, color, and style of drawing brush.

Text color

User can configure color used to draw fonts.
