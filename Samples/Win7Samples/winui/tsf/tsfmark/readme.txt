Text Services Framework Mark Sample Readme
==========================================

This sample demonstrates a few basic TSF interactions.  The code
implements a simple text service which can start a composition,
set property values, or set compartment values.  Property
serialization and unserialization is also demonstrated.

Target audience is text service writers.


Compiling
==========================================
To build Mark, use the nmake command.


Installation
==========================================

Mark will not run without the Text Services Framework installed,
which ships with WindowsXP and Microsoft OfficeXP.

Mark is a COM server dll.  After building Mark.dll, run

regsvr32 Mark.dll

from the command line to register it with the system.

Running
==========================================
Mark will be loaded into any text service framework aware
application, such as the sample application in this sdk or Microsoft
Word XP.  With Mark or any other text service installed, you will
see a floating "language bar" toolbar appear on the desktop.
If english is not your default language, use the toolbar to select
english, then select "Mark Text Service" from the dropdown.

From the "Mark Menu" on the language toolbar, you can run a few
simple property and composition demonstrations.
