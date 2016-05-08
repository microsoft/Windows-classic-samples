Text Services Framework Case Sample Readme
==========================================

This sample demonstrates a few basic TSF interactions.  The code
implements a simple text service which can insert text, or change
the english capitalization of existing text.

Target audience is text service writers.


Compiling
==========================================
To build Case, use the nmake command.


Installation
==========================================

Case will not run without the Text Services Framework installed,
which ships with WindowsXP and Microsoft OfficeXP.

Case is a COM server dll.  After building case.dll, run

regsvr32 case.dll

from the command line to register it with the system.

Running
==========================================
Case will be loaded into any text service framework aware
application, such as the sample application in this sdk or Microsoft
Word XP.  With Case or any other text service installed, you will
see a floating "language bar" toolbar appear on the desktop.
If english is not your default language, use the toolbar to select
english, then select "Case Text Service" from the dropdown.

From the "Case Menu" on the language toolbar, you can insert text
or toggle the capitalization of text in a document via the text service.
