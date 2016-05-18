Copyright 1993-1999 Microsoft Corporation. All rights reserved

The PROPERTY sample demonstrates the usage of three different styles of
property sheets: modal, modeless and wizard.

The main problem in dealing with a modeless property sheet is knowing
when to destroy the property sheet. This is handled in the
application's main message loop using the PSM_GETCURRENTPAGEHWND
message (PropSheet_GetCurrentPageHwnd macro). PSM_GETCURRENTPAGEHWND
will return NULL after the OK or Cancel buttons have been pressed, or
the dialog has been closed, and all of the pages have been notified. The
application can then call DestroyWindow on the window handle that was
returned from the PropertySheet call.

The sample also demonstrates the usage of the PropSheetCallback function
to modify the property sheet dialog before or after it is created. The
Wizard property sheet is modified to contain a system menu.
