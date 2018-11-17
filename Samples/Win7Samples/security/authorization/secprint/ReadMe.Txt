SecPrint.C

This sample illustrates how to use the low-level access control APIs to add
permissions to a printer.

Note that the specific combination of parameters shown below are not the
only possible combination to set on a printer object. The combination below
is the only combination, however, that will be recognized by the printer
control panel as security privileges that correspond to "print access",
"manage documents", "manage printer", and "full control" (and "no access"
on NT4 or earlier.)

Command line arguments: <printer name> <user name> <p|md|mp|f> [d]

 Where P  = Print access
       MD = Manage documents
       MP = Manage printers
       F  = Full control <P + MD + MP>

Use <D> to deny specified access to the specified group or user (otherwise
specified access is granted.)

Note: On NT4 or earlier denied 'Full Control' is displayed as 'No Access'
by the Explorer Printer Permissions dialog. Denying P, MD, or MP on NT4 or
earlier will not be viewable using this dialog even though the DACL is valid.
